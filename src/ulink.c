/*.
 *	ulink.c - remove unneeded link/unlink couples
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

/*.
 *	proc_label() - is ip a beginning of a function (label begins
 *		       with '_' or '@')
 */
static bool proc_label(ip)
	INST *ip;
	{
	char *s;
	ifn(s = ip->label) return FALSE;
	return *s=='_' || *s=='@';
	}

/*.
 *	u_optim() - optimize & return next label
 */
/*
 *	_LABEL1	...		=>	_LABEL1	...
 *		link	Am,X			<deleted>
 *		...				...
 *		<stuff>				<stuff>
 *		...				...
 *		unlink	Am			<deleted>
 *		...				...
 *	_LABEL2	...			_LABEL2	...
 *
 * Where Am is not used in <stuff>.
 */
static INST *u_optim(ibeg)
	INST *ibeg;
	{
	INST *ilink,*iunlink,*ip;
	
	/*.
	 *	find link
	 */
	if(ibeg->opcode == LINK) ilink = ibeg;
	else for(ilink = ibeg->next; ilink && !proc_label(ilink) &&
			  ilink->opcode!=LINK; ilink=ilink->next);
	if(!ilink || ilink->opcode!=LINK) return ilink;

	/*.
	 *	find unlink
	 */
	for(iunlink = ilink->next; iunlink && !proc_label(iunlink) && 
			  iunlink->opcode!=UNLK; iunlink=iunlink->next);
	if(!iunlink || iunlink->opcode!=UNLK) return iunlink;

	/*.
	 *	check mismatch
	 */
	if(ilink->src.areg != iunlink->src.areg) 
		{
		printf("%s: Warning link/unlink mismatch !\n",ProgName);
		dumpline(ilink);
		dumpline(iunlink);
		return iunlink;
		}

	/*.
	 *	check usage
	 */
	for(ip=ilink->next; ip != iunlink; ip=ip->next)
		if(ip->opcode != JSR && ip->opcode != BSR)
		{
		if(uses_slow(ip,ilink->src.areg)) return iunlink;
		}

	++s_link;
	DBG(printf("(%s) ",ibeg->label);fflush(stdout));

	ip = iunlink->next;
	delinst(ilink);
	delinst(iunlink);
	return ip;
	}

/*.
 * ulink() - process optimisation
 */
void ulink()
	{
	register INST	*ip;

	DBG(printf("ulink    : "));
	ip = head;
	while(ip)
		{
		while(ip && !proc_label(ip)) ip=ip->next;
		if(ip) ip = u_optim(ip);
		}
	DBG(printf("\n"));
	}
