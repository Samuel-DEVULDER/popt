/*
 *	instbis.c - Routines dealing with the parsing and output of 
 *		 instructions. Part II
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

/*
 * clearinst() - clear inst fields
 */
static void clearinst(ip)
	register INST *ip;
	{
	ip->label  = NULL;
	ip->rest   = NULL;
	ip->flags  = 0;
	ip->opcode = NO_INST;
	ip->next   = NULL;
	ip->prev   = NULL;
	ip->nb_hat =
	ip->lineno =
	ip->refs   =
	ip->sets   = 0;
	ip->live   = -1;	/* for newly inserted inst in peep opt */

	ip->src.areg  = ip->dst.areg  =
	ip->src.ireg  = ip->dst.ireg  =
	ip->src.disp  = ip->dst.disp  = 0;
	ip->src.amode = ip->dst.amode = NONE;
	}

/*
 * newinst() - return a newly allocated inst
 */
static INST *newinst()
	{
	register INST *ip;

	NEW(ip);
	clearinst(ip);
	return ip;
	}

/*
 * delinst(ip) - delete instruction pointed by ip
 */
void delinst(ip)
	register INST *ip;
	{
	register INST *ni;

	++s_idel;

	freeop(&ip->src);
	freeop(&ip->dst);
	/*
	 *	just clear the opcode if instruction bears a label...
	 */
	if(ip->label || (ip->rest && *ip->rest==';' && keepcomment))
		{
		ip->src.amode =
		ip->dst.amode = NONE;
		ip->opcode    = NO_INST;
		ip->flags     =
		ip->refs      =
		ip->sets      = 0;
		return;
		}
	/*
	 *	otherwise del instruction
	 */
	if(ip==head) head = ip->next;
	if(ni = ip->next) ni->prev = ip->prev;
	if(ni = ip->prev) ni->next = ip->next;
	if(ip->label) free(ip->label);
	if(ip->rest)  free(ip->rest);
	free(ip);
	}

/*
 * insinst(ip) - insert an instruction after ip;
 */
INST *insinst(ip)
	register INST	*ip;
	{
	register INST	*ni;

	ni = newinst();
	/*
	 * Link into the block appropriately
	 */
	if(ni->prev = ip)
		{
		if(ni->next = ip->next) ni->next->prev = ni;
		ip->next = ni;
		}

	return ni;
	}

/*
 * inspreinst(ip) - insert an instruction before ip;
 */
INST *inspreinst(ip)
	register INST	*ip;
	{
	register INST	*ni;

	ni = newinst();
	ni->label  = ip->label;
	ip->label  = NULL;
	ni->rest   = ip->rest;
	ip->rest   = NULL;
	ni->nb_hat = ip->nb_hat;
	ip->nb_hat = 0;
	/*
	 * Link into the block appropriately
	 */
	ni->next = ip;
	if(ip)
		{
		if(ip==head) head = ni;
		ni->prev = ip->prev;
		ip->prev = ni;
		if(ni->prev) ni->prev->next = ni;
		}

	return ni;
	}

/*
 * setuparg(op,p) - build operand
 */
static long *setupopnd(op,p)
	struct opnd *op;
	long *p;
	{
	if(*p==NONE) 
		{
		op->amode = *p++;
		}
	else if((*p&~SYMB)==IMM)
		{
		op->amode = *p++;
		op->disp  = *p++;
		}
	else if(*p==REG || (*p&~(INC|DEC))==REGI)
		{
		op->amode = *p++;
		op->areg  = *p++;
		}
	else if((*p&~XWORD)==(ABS|SYMB))
		{
		op->amode = *p++;
		op->astr  = (char*)*p++;
		}
	else if((*p&~XWORD)==ABS)
		{
		op->amode = *p++;
		op->disp  = (int)(char*)*p++;
		}
	else if((*p&~SYMB)==REGID)
		{
		op->amode = *p++;
		op->disp  = *p++;
		op->areg  = *p++;
		}
	else if((*p&(~(XLONG|SYMB)))==REGIDX)
		{
		op->amode = *p++;
		op->disp  = *p++;
		op->areg  = *p++;
		op->ireg  = *p++;
		}
	else if((*p&~SYMB)==PCD)
		{
		op->amode = *p++;
		op->disp  = *p++;
		}
	else if((*p&~(XLONG|SYMB))==PCDX)
		{
		op->amode = *p++;
		op->disp  = *p++;
		op->ireg  = *p++;
		}
	else if(*p==MSK)
		{
		op->amode = *p++;
		op->disp  = *p++;
		}
	else if(*p==STRNG)
		{
		op->amode = *p++;
		op->disp  = *p++;
		}
	else	return NULL;
	return p;
	}

/*
 * setupinst(ip, ...) - build ip structure
 */
#ifdef __GNUC__
INST *setupinst(INST *ip, ...)
#else
INST *setupinst(ip, ...)
	INST *ip;
#endif
	{
	long *p=(long*)&ip;

	++p;
	if(*p<0 || *p>UNKWN) 
		{
		fprintf(stderr,"%s: setupinst() - Unknown opcode! %d\n",
			ProgName,*p);
		exit(1);
		}
	ip->opcode = *p++;
	if(*p==NONE) {ip->flags = 0;++p;}
	else if(*p==LENL || *p==LENW || *p==LENB) ip->flags = *p++;
	ifn(p=setupopnd(&ip->src,p))
		{
		fprintf(stderr,"%s: setupinst() - Unknown src mode! 0x%0x\n",
			ProgName,*p);
		exit(1);
		}
	ifn(p=setupopnd(&ip->dst,p))
		{
		fprintf(stderr,"%s: setupinst() - Unknown dst mode! 0x%0x\n",
			ProgName,*p);
		exit(1);
		}
	ip->refs=ip->sets=ip->live=-1;
	return ip;
	}

/*
 * opeq(op1, op2) - test equality of the two instruction operands
 */
bool opeq(op1, op2)
	register struct	opnd	*op1, *op2;
	{
	if(op1->amode != op2->amode) return FALSE;
	switch (op1->amode & ~(X2|X4|X8))
		{
		case NONE: return TRUE;

		case REGI: case REGI|DEC: case REGI|INC:
		case REG: return op1->areg==op2->areg;
		
		case ABS: case ABS|XWORD:
		case PCD: case IMM: case MSK: return op1->disp==op2->disp;

		case PCD|SYMB: case STRNG: case ABS|SYMB: 
		case ABS|SYMB|XWORD: case IMM|SYMB:
		return !strcmp(op1->astr,op2->astr);
		
		case REGID:
		return op1->areg==op2->areg && op1->disp==op2->disp;

		case REGID|SYMB:
		return op1->areg==op2->areg && !strcmp(op1->astr,op2->astr);

		case REGIDX: case REGIDX|XLONG:
		return op1->areg==op2->areg && op1->disp==op2->disp &&
			op1->ireg==op2->ireg;

		case REGIDX|SYMB: case REGIDX|XLONG|SYMB:
		return op1->areg==op2->areg && !strcmp(op1->astr,op2->astr)
			&& op1->ireg==op2->ireg;

		case PCDX: case PCDX|XLONG:
		return op1->disp==op2->disp && op1->ireg==op2->ireg;

		case PCDX|SYMB: case PCDX|XLONG|SYMB:
		return !strcmp(op1->astr,op2->astr) && op1->ireg==op2->ireg;

		default:
		fprintf(stderr, "%s: opeq() - Bad addr. mode: %d\n",
			ProgName, op1->amode);
		return FALSE;
		}
	}


/*
 * Routines for printing out instructions
 */
static int npos(s,pos)
	register char *s;
	register int pos;
	{
	while(*s)
		{
		if(*s=='\t') {pos+=8;pos&=~7;}
		else if(*s=='\n') pos=0;
		else ++pos;
		++s;
		}
	return pos;
	}

void putinst(fp,ip)
	register INST	*ip;
	FILE		*fp;
	{
	register char	c;
	register int	i;
	static char	oprnd[20];
	register int	pos=0;

	for(i=0;i<ip->nb_hat;++i) {fprintf(fp,"^");++pos;}

	if(ip->label)   
		{
		fprintf(fp,"%s",ip->label);
		if(slflag && is_real_inst(ip->opcode))
			{
			fprintf(fp,"\n");
			pos = 0;
			}
		else pos+=strlen(ip->label);
		}

	if(ip->opcode == UNKWN)
		{
		fprintf(fp,"%s",ip->rest);
		pos=npos(ip->rest,pos);
		}
	else if(ip->opcode != NO_INST)
		{
		if(pos<8) {fprintf(fp,"\t");pos=8;}
		else	  {fprintf(fp," ");++pos;}

		fprintf(fp, "%s", opnames[ip->opcode]);
		pos+=strlen(opnames[ip->opcode]);
		switch(ip->flags)
			{
			case LENB:
			c = 'b';
			break;

			case LENW:
			c = 'w';
			break;

			case LENL:
			c = 'l';
			break;

			default:
			c = '\0';
			break;
			}
		if(c)   {fprintf(fp, ".%c", c);pos+=2;}

		if(ip->src.amode != NONE)
			{
			fprintf(fp, "\t");pos+=8;pos&=~7;
			ifn(putop(oprnd,&ip->src))
				{
				dumpline(ip);
				exit(1);
				}
			fprintf(fp, "%s", oprnd);
			pos+=strlen(oprnd);
			}
		if(ip->dst.amode != NONE)
			{
			fprintf(fp, ",");++pos;
			ifn(putop(oprnd,&ip->dst))
				{
				dumpline(ip);
				exit(1);
				}
			fprintf(fp, "%s", oprnd);
			pos+=strlen(oprnd);
			}
		if (ip->rest)
			{
			register char *s=ip->rest;
			if(pos >= 40) {fprintf(fp,"\n");pos = 0;}
			while(pos < 40) {fprintf(fp,"\t");pos+=8;pos&=~7;}
			while(*s=='\t') ++s;
			fprintf(fp, "%s", s);
			pos+=strlen(s);
			}
		}
	else if(ip->rest)
		{
		if(ip->label) {fprintf(fp," ");++pos;}
		fprintf(fp, "%s", ip->rest);
		pos=npos(ip->rest,pos);
		}
#ifdef	DEBUG
	if(debug && ip->opcode!=NO_INST && is_real_inst(ip->opcode))
		{
		if(pos >= 40) 
			{
			fprintf(fp,"\n");
			pos = 0;
			}
		while(pos < 40) {fprintf(fp,"\t");pos+=8;pos&=~7;}
		sprintf(oprnd, "; ref=%04x set=%04x live=%04x", ip->refs,ip->sets,ip->live);
		fprintf(fp,"%s", oprnd);
		pos+=strlen(oprnd);
		}
#endif
	fprintf(fp, "\n");
	}

/*
 * masktos() - convert a register mask to a descriptive string
 *
 * Generates a string of the form "Rm/Rn/Ro/..."
 */
char *masktos(mask)
	register int	mask;
	{
	static	char	buf[64];
	register char	*p = buf;
	register int	r, r1,r2;

	for(r = D0; r <= D7 ;++r)
		{
		if(mask & RM(r))
			{
			if (p != buf) *p++ = '/';
			r1   = r;
			while ((r<=D7) && (mask & RM(r))) ++r;
			r2   = r-1;
			r    = r1;
			*p++ = 'D';
			*p++ = (r - D0) + '0';
			if(r2-r1>1)
				{
				r = r2;
				*p++ = '-';
				*p++ = 'D';
				*p++ = (r - D0) + '0';
				}
			}
		}
	for(r = A0; r <= A7 ;++r)
		{
		if(mask & RM(r))
			{
			if (p != buf) *p++ = '/';
			r1   = r;
			while ((r<=A7) && (mask & RM(r))) ++r;
			r2   = r-1;
			r    = r1;
			*p++ = 'A';
			*p++ = (r - A0) + '0';
			if(r2-r1>1)
				{
				r = r2;
				*p++ = '-';
				*p++ = 'A';
				*p++ = (r - A0) + '0';
				}
			}
		}
	*p = '\0';

	return buf;
	}
