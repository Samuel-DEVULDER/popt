/*
 *	source.c - get source & put source...
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

INST *head;	/* global head of the asm sourcefile */

/*
 * putoptions() - put options..
 */
void putoptions(fp)
	FILE *fp;
	{
#ifdef	DEBUG
	if(debug)       fprintf(fp,"debug ");
#endif
	if(safe)        fprintf(fp,"safe ");
	if(size)        fprintf(fp,"size ");
	if(mc20)        fprintf(fp,"68020/030 ");
	if(mc40)        fprintf(fp,"68040 ");
	if(mc60)        fprintf(fp,"68060 ");
	if(newsn)       fprintf(fp,"new_syntax ");
	if(keeplbl)     fprintf(fp,"keep_label ");
	if(keepcomment) fprintf(fp,"keep_comment ");
	if(slflag)	fprintf(fp,"separate_lines ");
	if(nowarning)	fprintf(fp,"no_warning ");
	if(nostack)	fprintf(fp,"no_stack_fixup ");
	if(newinsts)	fprintf(fp,"newinsts ");
	}

/*
 * getsource() - get a source and return a pointer to the first instruction.
 *
 */
INST *getsource(name)
	char	*name;
	{
	register INST	*ci;	/* the block we're currently reading */
	FILE		*fp;
	char		*line;

	fp = safeopen(name, "r");

	head = NULL;
	ci   = NULL;
	
	while(line = readline(fp))
		{
		ci = addinst(ci, line);
		ifn(head) head = ci;
		}

	safeclose(fp);
	
	return head;
	}

/*
 * putsource() - write back source
 */
void putsource(name)
	char	*name;
	{
	register INST	*ci;
	FILE		*fp;
	
	fp=safeopen(name,"w");
	fprintf(fp,"; POPT OUTPUT \"%s\"\n; FLAGS=",name);
	putoptions(fp);
	fprintf(fp,"\n");
	if(debug)
		{
		fprintf(fp,"; call_refs=%s ",cr_regs?masktos(cr_regs):"''");
		fprintf(fp,"call_sets=%s ",cs_regs?masktos(cs_regs):"''");
		fprintf(fp,"used_rts=%s\n",used_rts?masktos(used_rts):"''");
		}
	fprintf(fp,"\n");

	for (ci = head; ci ;ci = ci->next) putinst(fp,ci);

	safeclose(fp);
	}
