/*
 *	labmerge.c - merging multiple labels to one single
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

/*
 *	islabel(ip) - is ip just a label
 */
static bool islabel(ip)
	INST *ip;
	{
	if(ip->nb_hat) return FALSE;
	if(ip->opcode!=NO_INST) return FALSE;
	if(ip->rest) return FALSE;
	return ip->label!=(char*)NULL;
	}

/*
 *	repl_opnd(lab1,lab2,opnd) - replace every occurence of lab1 by lab2 
 *	in operand pointed by opnd
 */
static void repl_opnd(lab1,lab2,op)
	char *lab1,*lab2;
	struct opnd *op;
	{
	if(is_astr(op->amode) && !strcmp(op->astr,lab1))
		{
		free(op->astr);
		op->astr = strsave(lab2);
		}
	}
/*
 *	replace(lab1,lab2) - replace every occurence of lab1 by lab2 in 
 *	code
 */
static void replace(lab1,lab2)
	char *lab1,*lab2;
	{
	INST *hp = head;
	while(hp)
		{
		repl_opnd(lab1,lab2,&hp->src);
		repl_opnd(lab1,lab2,&hp->dst);
		hp = hp->next;
		}
	}

/*
 *	addequ(ip,lab1,labl2) - add "lab1 equ lab2" statement at end of
 *		source for savety because of dc.l statements.
 */
static void addequ(ip,lab1,lab2)
	INST *ip;
	char *lab1,*lab2;
	{
	char *t;
	INST *ni;
	if(ip->next)
		{
		ip = ip->next;
		for(;(ni=ip->next) && ni->opcode!=EQU &&
		     ni->opcode!=END;ip=ni);
		}
	t=alloc(256);	/* should be enough 256 */
	sprintf(t,"%s\tequ\t%s",lab1,lab2);
	addinst(ip,t);
	free(t);
	}

/*
 *	labmerge() - merge multiple label together
 */
void labmerge()
	{
	INST *ip,*ni,*ti;

	DBG(printf("labmerge( )\010\010"))

	for(ip=head;ip;)
		{
		for(;ip && !islabel(ip);)
			{
			/*
			 * wip out empty lines
			 */
			if(!ip->label && !ip->nb_hat && ip->opcode==NO_INST &&
			   !ip->rest) 
				{
				INST *ni=ip->next;
				delinst(ip);
				ip=ni;
				--s_idel;
				}
			else ip=ip->next;
			}
		ifn(ip) goto end;
		for(ni=ip->next;ni && islabel(ni);ni=ti)
			{
			DBG(progress())
			++s_labmerge;
			addequ(ni,ni->label,ip->label);
			replace(ni->label,ip->label);
			ti = ni->next;free(ni->label);ni->label=NULL;
			delinst(ni);
			--s_idel;
			}
		/*
		 * Patch for things like:
		 *	lbl1:
		 *	lbl2	= lbl1-15
		 *  to prevent it from being transformed as
		 *  lbl1 = lbl1-15 because of label clobbering.
		 */
		if(ni && ni->label && (ni->opcode==EQU || ni->opcode==EQUAL||
			ni->opcode==SeT)) {ip=ni;continue;}
		if(ni && ni->label) 
			{
			++s_labmerge;
			addequ(ni,ni->label,ip->label);
			replace(ni->label,ip->label);
			ti = ni->next;free(ni->label);ni->label=NULL;
			}
		if(ni)	{
			DBG(progress())
			ni->label = ip->label;
			ip->label = NULL;
			ti = ip->next;
			delinst(ip);
			--s_idel;
			ip = ti;
			}
		else	ip = ip->next;
		}
end:	DBG(printf(") \n"))
	}
