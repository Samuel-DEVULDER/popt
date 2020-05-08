/*.
 *	braopt2.c - branch optimization (branch to branch & branch to next,
 *	branch inversion, dbf optimizations). Part II.
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"
#include <strings.h>

#define	LAB_TOUCHED	0x80
#define	OP_BRANCH(x)	(((x)->opcode==BRA || (x)->opcode==JMP) && \
			 (((x)->src.amode&~XWORD)==(ABS|SYMB) || \
			  (x)->src.amode==(PCD|SYMB)))

/*.
 *	find_label(label) - find the label in code
 */
INST *find_label(label)
	char *label;
	{
	INST *hp = head;
	while(hp)
		{
		if(hp->label && !strcmp(hp->label,label)) return hp;
		hp = hp->next;
		}
	return NULL;
	}

/*.
 *	clear_touched() - clear marks
 */
void clear_touched()
	{
	INST *hp = head;
	while(hp)
		{
		for(;hp && !hp->label;hp=hp->next);
		if(hp) {hp->flags &= ~LAB_TOUCHED;hp=hp->next;}
		}
	}
	
/*.
 * bcomp(bcc) - return the complement of the given branch code
 *
 * Used when a branch reversal is needed.
 */
int bcomp(bcc)
	register int	bcc;
	{
	switch (bcc)
		{
		case BHI: return BLS;
		case BLS: return BHI;
		case BCC: return BCS;
		case BCS: return BCC;
		case BNE: return BEQ;
		case BEQ: return BNE;
		case BVC: return BVS;
		case BVS: return BVC;
		case BPL: return BMI;
		case BMI: return BPL;
		case BGE: return BLT;
		case BLT: return BGE;
		case BGT: return BLE;
		case BLE: return BGT;
		
		default:  return 0;
		}
	}

/*.
 * scomp(scc) - return the complement of the given s<cc> code
 */
int scomp(scc)
	register int	scc;
	{
	switch (scc)
		{
		case SHI: return SLS;
		case SLS: return SHI;
		case SCC: return SCS;
		case SCS: return SCC;
		case SNE: return SEQ;
		case SEQ: return SNE;
		case SVC: return SVS;
		case SVS: return SVC;
		case SPL: return SMI;
		case SMI: return SPL;
		case SGE: return SLT;
		case SLT: return SGE;
		case SGT: return SLE;
		case SLE: return SGT;
		
		default:  return 0;
		}
	}

/*.
 * binv(bc) - return the inverse of the given branch code. give '<' for '>'.
 */
int binv(bc)
	register int	bc;
	{
	switch (bc)
		{
		case BHI: return BLT;
		case BLS: return BGE;
		case BCC: return BCC;
		case BCS: return BCS;
		case BEQ: return BEQ;
		case BNE: return BNE;
		case BVC: return BVC;
		case BVS: return BVS;
		case BPL: return BMI;
		case BMI: return BPL;
		case BGE: return BLE;
		case BLT: return BGT;
		case BGT: return BLT;
		case BLE: return BGE;
		
		default:  return 0;
		}
	}

/*.
 * evaltst(bcc,size,nb) - evaluate test. 3 is returned is nothing can be said.
 */
int evaltst(bc,size,nb)
	register int	bc,size,nb;
	{
	if(size==LENB) nb=(char)nb;
	else if(size==LENW) nb=(short)nb;

	switch (bc)
		{
		case BHI: return 3;
		case BLS: return 3;
		case BCC: return 3;
		case BCS: return 3;
		case BNE: return nb!=0;
		case BEQ: return nb==0;
		case BVC: return 3;
		case BVS: return 3;
		case BPL: return 3;
		case BMI: return 3;
		case BGE: return nb>=0;
		case BLT: return nb<0;
		case BGT: return nb>0;
		case BLE: return nb<=0;
		
		default:  return 3;
		}
	}

/*.
 *	i_isused(lbl,ip) - internal func..
 */
static bool i_isused(lbl,ip)
	char *lbl;
	INST *ip;
	{
	char str[100],*s,*t;
	int i,t1,t2;

	t1=is_astr(ip->src.amode);
	t2=is_astr(ip->dst.amode);
	if(ip->rest || t1 || t2)
		{
		s = str;i=0;
		if(t1)	{
			t = ip->src.astr;
			while(*t && i<99) {*s=*t;++s;++t;++i;}
			if(i<99) {*s=' ';++s;++i;}
			}
		if(t2)	{
			t = ip->dst.astr;
			while(*t && i<99) {*s=*t;++s;++t;++i;}
			}
		if(t=ip->rest) while(*t && i<99){*s=*t;++s;++t;++i;}
		*s='\0';
		if(i==99) 
			{
			fprintf(stderr,	"%s: islabelused() - Warning, line "
				"too long: \"%s\"\n",ProgName,str);
			}
		ifn(t=strstr(str,lbl)) return FALSE;
		t+=strlen(lbl);
		if((*t>='0' && *t<='9') || /* *t=='.' || */ *t=='_' || *t=='@' ||
		   (*t>='a' && *t<='z') || (*t>='A' && *t<='Z')) 
			return FALSE;
		return TRUE;
		}
	return FALSE;
	}

/*.
 *	isused(lbl) - is label used
 */
bool islabelused(lbl)
	char *lbl;
	{
	INST *ip;
	static INST *lh=NULL;

	/* to speed up research, we keep the last occurence in lh, because
	   statically the next occurence is likely to occur after that one */
/*fprintf(stderr,"islabelused:%s\n",lbl);	*/
	/*.
	 *	by default, proc labels are always used... 
	 */
	if(*lbl=='_' || *lbl=='@') return TRUE;
	/*.
	 *	else really check.
	 */
	ifn(lh) lh = head;
	for(ip=lh;ip;ip=ip->next) if(i_isused(lbl,ip))
		{
		lh=ip;
		return TRUE;
		}
	for(ip=lh;ip;ip=ip->prev) if(i_isused(lbl,ip))
		{
		lh=ip;
		return TRUE;
		}
	return FALSE;
	}

/*.
 *	cleanup() - clear unused label
 */
void cleanup()
	{
	INST *ip;
	char *l;
	bool changed,printed = FALSE;

	do
		{
		changed = FALSE;
		for(ip=head;ip;ip=ip->next) 
			{
			if((l=ip->label) && !islabelused(l))
				{
				if(ip->opcode == EQU   || ip->opcode == EQUR || 
				   ip->opcode == EQUAL || ip->opcode == ReG || 
				   ip->opcode == SeT )
				   	{
					INST *ni=ip->prev;
				   	delinst(ip);
				   	ifn(ip = ni) break;
				   	}
				else	{
					free(l);
					ip->label = NULL;
					}
				changed = TRUE;
				if(!printed) 
					{
					printed = TRUE;
					DBG(printf("clearing_unused_labels( )\010\010");fflush(stdout))
					}
				DBG(progress())
				}
			}
		}
	while(changed);
	if(printed) DBG(printf(") ");fflush(stdout))
	}

/*.
 *	del_unreach() - delete some unreachable code.
 */
void del_unreach()
	{
	INST *ip;
	bool jump_table;

	for(ip=head;ip;) 
		{INST *ni;int m;
		/*.
		 *	hack to detect table of jumps.
		 */
		if(ip->label && (*ip->label=='_'||*ip->label=='@')) 
			jump_table = FALSE;
		m = M(ip->src.amode);
		if(ip->opcode == JMP && 
		   (m==PCDX || m==REGI || m==REGID || m==REGIDX))
		   	{
		   	jump_table = TRUE;
		   	if(ip->lineno)
			DBG(printf("<%d> ",ip->lineno);fflush(stdout);)
		   	}
		/*
		 *	Delete code never reached: Such a code is a
		 *	code following an inconditionnal jump that
		 *	bears no label. This is not true for a series
		 *	of	jmp	lbl1(pc)
		 *		jmp	lbl2(pc)
		 *	that is used for jump tables.
		 */
		if(!jump_table && OP_BRANCH(ip) && (ni = ip->next) && 
		   is_real_inst(ni->opcode) && !ni->label)
			{
			/*.
			 * code never reached
			 */
			DBG(printf("bnl(%d) ",ip->lineno);fflush(stdout))
			do	{
				ni=ni->next;
				delinst(ip->next);
				}
			while(ni && is_real_inst(ni->opcode) && !ni->label);
			if(ni->label && !strcmp(ip->src.astr,ni->label))
				{
				delinst(ip);
				ifn(islabelused(ni->label)) 
					{
					free(ni->label);
					ni->label = NULL;
					}
				}
			ip = ni;
			}
		else ip=ip->next;
		}
	DBG(printf(" \n"));
	}	
