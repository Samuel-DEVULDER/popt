/*.
 *	stack.c - Stack optimisation. 
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

int s_stack;

/*. 
 * 	syncpoint() - check if instruction is a syncpoint.
 * 
 * We synchonise on ip if:
 *	- ip bears a label
 *	- ip is a branch (conditional/unconditional)
 *	- ip is rts, rte, ...
 *	
 */
static bool syncpoint(INST *ip)
	{
	int op=ip->opcode;
	if(ip->label) return TRUE;
	if(isbranch(op)) return TRUE;
	switch(op)
		{
		case RTS: case RTE: case RTR: case JMP: return TRUE;
		default: return FALSE;
		}
	}

/*
 *	stackopt() - Tries to merge multiples add #n,sp together.
 */
bool stackopt()
	{
	INST *ip,*add;
	bool modified = FALSE;
	
	if(nostack) return FALSE;

	add = NULL;
	for(ip=head;ip;ip=ip->next)
		{
		if(syncpoint(ip)) add = NULL;

		if(( (OP_ADD(ip->opcode) && ip->src.amode==IMM)
		   ||(ip->opcode==LEA && ip->src.amode==REGID &&
		      ip->src.areg==SP)
		   ) && 
		   ip->src.disp > 0 &&
		   ip->dst.amode==REG && ip->dst.areg==SP)
		   	{
		   	ifn(add) add = ip;
		   	else
		   		{
		   		ip->src.amode = IMM;
		   		ip->src.disp += add->src.disp;
		   		ip->opcode    = (ip->src.disp <= 8)?ADDQ:ADD;
				ip->flags     = (mc20 || mc40)?LENL:LENW;

				delinst(add);
				add = ip;

		   		ifn(modified) 
		   			{
		   			modified = TRUE;
					DBG(printf("stackopt : "));
		   			}
		   		DBG(printf("%d ",ip->lineno);fflush(stdout));
		   		++s_stack;
		   		}
		   	}
		}
	
	if(modified) DBG(printf("\n"));
	return modified;
	}
