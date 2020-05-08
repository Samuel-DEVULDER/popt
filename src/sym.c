/*
 *	sym.c - symbol managment module
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

/*
 * freeop() - free an operand
 */
void freeop(op)
	struct	opnd	*op;
	{
	if(is_astr(op->amode) && ((void*)op->astr) != NULL) free(op->astr);
	}

/*
 * freesym() - free all symbol table space
 */
void freesym()
	{
	register INST	*ip, *nexti;

	for(ip = head; nexti = ip->next; ip = nexti);

	for(; ip ; ip = nexti)
		{
		nexti = ip->prev;

/*		putinst(stdout, ip);	/* to check... */
		freeop(&ip->src);
		freeop(&ip->dst);
		if(ip->label) free(ip->label);
		if(ip->rest)  free(ip->rest);
		free(ip);
		}
	}

char *mktmp()
	{
	static	char	tname[16];
	static	short	tnum = 0;

	sprintf(tname, "POPT%d", tnum++);

	return tname;
	}
