/*
 *	movem.c - put good register masks for movem instructions
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

/*
 *	finddef(astr) - find the definition of astr in the prog by
 *	=, equ or equr or reg instruction.
 */
static char *finddef(var)
	char *var;
	{
	INST *ip;
	static char buff[40];
	for(ip = head; ip; ip=ip->next)
	if(ip->label && !strcmp(ip->label, var) && 
	  (ip->opcode == EQU || ip->opcode == EQUR || 
	   ip->opcode == EQUAL || ip->opcode == ReG ))
		{
		putop(buff,&ip->src);
		return buff;
		}

	return NULL;
	}

/*
 *	get_reg_mask(ip,opnd)
 */
static int get_reg_mask(ip,op)
	INST		*ip;
	struct opnd	*op;
	{
	int mask;
	char *def;

	if(op->amode == REG) mask = RM(op->areg);
	else if(op->amode == MSK) mask = op->disp;
	else if(M(op->amode) != ABS)
		{
		fprintf(stderr,"%s: get_reg_mask() - Bad mode %d\n",
			ProgName,op->amode);
		dumpline(ip);
		exit(1);
		}
	else	{
		if(def = finddef(op->astr))
			{
			mask = stomask(def);
			if(*def && !mask)
				{
				fprintf(stderr,
			"%s: get_reg_mask() - Not a register mask: \"%s\"\n",
					ProgName, def);
				dumpline(ip);
				exit(1);
				}
			}
		else	{
			fprintf(stderr,
		"%s: get_reg_mask() - Can't find definition of \"%s\"\n",
			ProgName, op->astr);
			dumpline(ip);
			exit(1);
			}
		}

	return mask;
	}

/*
 * fix_movem() - fix reg mask of movem instructions
 */
void fix_movem()
	{
	INST *ip;
	int m;
	DBG(printf("fix_movem( )\010\010"))
	for(ip=head;ip;ip=ip->next) if(ip->opcode==MOVEM)
		{
		if((m = M(ip->dst.amode)) == ABS || m == STRNG || m == MSK || 
		   m == REG)
			{
			m = get_reg_mask(ip,&ip->dst);
			freeop(&ip->dst);
			ip->dst.amode = MSK;
			ip->dst.disp  = m;
			DBG(progress())
			}
		else if((m = M(ip->src.amode)) == ABS || m == STRNG ||
			 m == MSK || m == REG)
			{
			m = get_reg_mask(ip,&ip->src);
			freeop(&ip->src);
			ip->src.amode = MSK;
			ip->src.disp  = m;
			DBG(progress())
			}
		else	{
			fprintf(stderr,
			"%s: fix_reg_mask() - Strange movem !\n",ProgName);
			dumpline(ip);
			exit(1);
			}
		}
	DBG(printf(") \n"))
	}

static bool fail;
/*
 * read a number. set fail.
 */
static int getnum(s)
	char *s;
	{
	bool neg = FALSE;
	bool hex = FALSE;
	int  num = 0;
	int  dig;

	fail = FALSE;
	if(*s=='-') {++s;neg = TRUE;}
	if(*s=='+') {++s;if(neg) goto failed;}
	if(*s=='$') {++s;if(!hex) hex = TRUE; else goto failed;}
	for(;*s;++s)
		{
		if(*s>='0' && *s<='9') dig = *s-'0';
		else if(hex && *s>='a' && *s<='f') dig = *s-'a'+10;
		else if(hex && *s>='A' && *s<='F') dig = *s-'A'+10;
		else goto failed;
		if(hex) num <<= 4;
		else	{
			num +=  num<<2;
			num <<= 1;
			}
		num +=  dig;
		}
	if(neg) num = -num;
	return num;
failed: fail = TRUE;
	return 0;
	}

/*
 * fix_link() - fix link instructions
 */
void fix_link()
	{
	INST *ip;
	DBG(printf("fix_link( )\010\010"))
	for(ip=head;ip;ip=ip->next) if(ip->opcode==LINK)
		{
		if(ip->dst.amode==(IMM|SYMB))
			{char *expr=ip->dst.astr,*def;
			 bool neg=FALSE;

			while(*expr=='-' || *expr=='+') 
				{
				if(*expr=='-') neg=~neg;
				++expr;
				}
			if(def = finddef(expr))
				{
				int n = getnum(def);
				if(!fail)
					{
					free(ip->dst.astr);
					ip->dst.amode = IMM;
					ip->dst.disp  = neg ? -n:n;
					DBG(progress());
					}
				}
			}
		}
	DBG(printf(") \n"))
	}
