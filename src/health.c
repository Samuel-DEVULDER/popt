/*
 *	health.c - Routines for data flow analysis by block and function.
 *		   A block is a part of code executed without any jumps.
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

#define	B_MARK		1
#define	B_RET		2
#define	B_STRANGE	4		/* jmp (a0,d0) */

typedef struct block
	{
	struct block *next,		/* next block in file. */
		     *bfall,		/* block we go through after this */
					/* block. */
		     *bcond;		/* block reached through a */
					/* conditional branch. */
	INST 	     *start,		/* first instr in block */
		     *end;		/* last instr in block */
	unsigned short 
		      refs,sets,	/* regs refs & sets by this block */
		      flags;
	} BLOCK;

/*
 *	chkfardata(s) - check if s has a pseudo inst for setting far data
 *			model. (actually search for inst 'far' with 'data' 
 *			arg).
 */
static bool chkfardata(s)
	register char *s;
	{
	register char *t;
	t="data";
	while(*t && (*s==*t)) {++s;++t;}
	if(*t) return FALSE;
	return TRUE;
	}

/*
 *	chkxref() - check if argument is an external ref
 */
bool chkxref(s)
	char *s;
	{
	INST *ip;
	for(ip=head;ip;ip=ip->next) 
		if((ip->opcode==XREF || ip->opcode==PUBLIC) && 
			is_astr(ip->src.amode) && !strcmp(s,ip->src.astr))
			return TRUE;
	return FALSE;
	}

/*
 *	freeblocks() - cleanup everything for this module
 */
static void freeblocks(bh)
	register BLOCK *bh;
	{
	register BLOCK *b2;
	while(bh)
		{
		b2 = bh->next;
		DELETE(bh);
		bh = b2;
		}
	}

/*
 *	putblocks() - output blocks list onto stdout... for debugging.
 */
static BLOCK *putblocks(bh)
	register BLOCK *bh;
	{
	register INST *ip;
	while(bh)
		{
		printf("; Block ref=%04x set=%04x flg=%02x\n",bh->refs,bh->sets,bh->flags);
		for(ip=bh->start; ip!=bh->end; ip=ip->next) 
			putinst(stdout,ip);
		putinst(stdout,ip);
		bh=bh->next;
		}
	return NULL;
	}

/*
 *	getblock() - returns block whose name is label
 */
static BLOCK *getblock(label,bh)
	register char *label;
	register BLOCK *bh;
	{
	register INST *ip;
	while(bh)
		{
		ip=bh->start;
		if(ip->label && !strcmp(ip->label,label)) return bh;
		bh=bh->next;
		}
	return NULL;
	}

/*
 *	isbranch() - is this opcode a branch opcode ?
 */
int isbranch(op)
	int op;
	{
	switch(op)
		{
		case BCC: case BCS: case BEQ: case BGE:
		case BGT: case BHI: case BLE: case BLS:	
		case BLT: case BMI: case BNE: case BPL:	
		case BRA: case BVC: case BVS:
		return 1; 

		case DBCC: case DBCS: case DBEQ: case DBF:
		case DBGE: case DBGT: case DBHI: case DBLE:
		case DBLS: case DBLT: case DBMI: case DBNE:
		case DBPL: case DBRA: case DBT:  case DBVC:
		case DBVS:
		return 2;
		
		default: return 0;
		}
	}

/*
 *	breakflow(ip) - does ip break the flow of the prog (ie. is a 
 *	branch or bear a label).
 */
int breakflow(ip)
	INST *ip;
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
 *	ubreakflow(ip) - does ip always break the flow of the prog 
 *	(ie. is a unconditional branch or bear a label).
 */
int ubreakflow(ip)
	INST *ip;
	{
	int op=ip->opcode;
	if(ip->label) return TRUE;
	switch(op)
		{
		case BRA:
		case RTS: case RTE: case RTR: case JMP: return TRUE;
		default: return FALSE;
		}
	}

/*
 *	db_cc(b_cc) - return the db<cc> version of the branch
 */
int db_cc(b_cc)
	int b_cc;
	{
	switch(b_cc)
		{
		case BCC: return DBCC;
		case BCS: return DBCS;
		case BEQ: return DBEQ;
		case BGE: return DBGE;
		case BGT: return DBGT;
		case BHI: return DBHI;
		case BLE: return DBLE;
		case BLS: return DBLS;
		case BLT: return DBLT;
		case BMI: return DBMI;
		case BNE: return DBNE;
		case BPL: return DBPL;
		case BRA: return DBF;
		case BVC: return DBVC;
		case BVS: return DBVS;
		
		default: return 0;
		}
	}

/*
 *	bcut() - cut source into blocks.
 */
BLOCK *bcut()
	{
	register INST *ip;
	register BLOCK *bh=NULL,*cp;
	bool found;

	ip = head;
	NEW(bh);cp=bh;

	while(ip)
		{
		cp->start = ip;
		cp->end   = NULL;
		cp->next  = NULL;
		cp->flags = 0;
		cp->refs  = 0;
		cp->sets  = 0;
		found     = FALSE;
		do	{
			if(isbranch(ip->opcode)) found=TRUE;
			else switch(ip->opcode)
				{
				case RTS: case RTE: case RTR:
				cp->flags   = B_RET;
				found       = TRUE;
				break;

				case JMP:
				cp->flags   = (M(ip->src.amode)!=ABS)?
								B_STRANGE:0;
				found       = TRUE;
				break;
				}
			ifn(ip->next) found = TRUE; 
			ifn(found)
				{
				ip = ip->next;
				if(ip->label)
					{
					ip    = ip->prev;
					found = TRUE;
					}
				}
			}
		while(!found);
		cp->end = ip;
		if(ip = ip->next)
			{
			NEW(cp->next);
			cp = cp->next;
			}
		}
	return bh;
	}

/*
 *	bfix() - setup bfall & bcond of blocks
 */
static void bfix(bh)
	register BLOCK *bh;
	{
	register BLOCK *cp,*np;
	register INST  *ip;

	for(cp = bh; cp; cp=cp->next)
		{
		if(cp->flags & B_STRANGE) 
			{
			cp->bfall = cp->bcond = NULL;
			continue;
			}
		ip = cp->end;
		switch(isbranch(ip->opcode))
			{
			case 1:
			if(M(ip->src.amode) == ABS)
				{
				ifn(np=getblock(ip->src.astr,bh))
					{
					if(chkxref(ip->src.astr))
						cp->flags |= B_STRANGE;
					else	{
						fprintf(stderr,
"%s: bfix() - Branch to bad label \"%s\" !\n",ProgName,ip->src.astr);
						dumpline(ip);
						exit(1);	/**/
						}
					}
				}
			else	np = NULL;
			if(ip->opcode == BRA)
				{
				cp->bfall = np;
				cp->bcond = NULL;
				}
			else
				{
				cp->bcond = np;
				cp->bfall = cp->next;
				}
			break;
			
			case 2:
			if(M(ip->dst.amode) == ABS)
				{
				ifn(np=getblock(ip->dst.astr,bh))
					{
					if(chkxref(ip->dst.astr))
						cp->flags |= B_STRANGE;
					else	{
						fprintf(stderr,
"%s: bfix() - Branch to bad label \"%s\" !\n",ProgName,ip->dst.astr);
						dumpline(ip);
						exit(1);	/**/
						}
					}
				}
			else	np = NULL;
			cp->bcond = np;
			cp->bfall = cp->next;
			break;
			
			default:
			if(ip->opcode == JMP)
				{int m = M(ip->src.amode);
				if(m == REGIDX)
					{
					cp->flags |= B_STRANGE;
					cp->bfall = cp->bcond = NULL;
					continue;
					}
				if(m == ABS || m == REGID)
					{
					ifn(np=getblock(ip->src.astr,bh))
						{
						if(chkxref(ip->src.astr))
							cp->flags |= B_STRANGE;
						else	{
							fprintf(stderr,
"%s: bfix() - Branch to bad label \"%s\" !\n",ProgName,ip->src.astr);
							dumpline(ip);
							exit(1);	/**/
							}
						}
					}
				else	np = NULL;
				cp->bfall = np;
				cp->bcond = NULL;
				}
			else
				{
				cp->bcond = NULL;
				cp->bfall = cp->next;
				}
			break;
			}
		}
	}

/*
 *	chka4() - if the prog is in near data model, mark a4 as beeing used
 *		  by all instructions.
 */
static void chka4()
	{
	register INST *ip;
	register bool fardata;
	register int RMA4=RM(A4);
	fardata = FALSE;
	for(ip = head; ip && !fardata; ip=ip->next) 
		{
		if(ip->opcode == FAR && ip->rest) 
			fardata = chkfardata(ip->rest);
		}
	if(!fardata) for(ip = head; ip; ip=ip->next) 
		if(M(ip->src.amode) == ABS || M(ip->dst.amode) == ABS)
			ip->refs |= RMA4;
	}

/*
 * bprep() - initial analysis of a block
 */
static void bprep(bp)
	BLOCK	*bp;
	{
	register INST	*ip;
	register int	refs, sets;

	refs = sets = 0;
	for(ip = bp->start; ip!=bp->end ;ip = ip->next)
		{
		refs |= (ip->refs = reg_ref(ip)) & ~sets;
		sets |= (ip->sets = reg_set(ip));
		}
	refs |= (ip->refs = reg_ref(ip)) & ~sets;
	sets |= (ip->sets = reg_set(ip));

	bp->refs = refs;
	bp->sets = sets;
	}

/*
 * fprep() - perform initial analysis of individual blocks in a function
 */
static void fprep(bp)
	BLOCK	*bp;
	{
	register BLOCK	*cp;
	for(cp = bp; cp ;cp = cp->next) bprep(cp);
	}

/*
 *	prep(head) - initial analysis of sourcefile pointed by head
 */
static BLOCK *prep()
	{
	register BLOCK *bh;
	bh = bcut();
	bfix(bh);
	fprep(bh);
	chka4();
	return bh;
	}

/*
 *	scan_ref() - is reg referenced within the block pointed by bp
 */
static bool scan_ref(bp, reg)
	register BLOCK	*bp;
	register int	reg;
	{
	register INST	*ip;

	if(bp == NULL)            return FALSE;
	if(bp->refs & reg)        return TRUE;
	if(bp->sets & reg)        return FALSE;	
	if(bp->flags & B_STRANGE) return TRUE;	
	if(bp->flags & B_MARK)    return FALSE;	
	if(bp->flags & B_RET)     return FALSE;

	bp->flags |= B_MARK;

	if(scan_ref(bp->bcond, reg)) return TRUE;
	if(scan_ref(bp->bfall, reg)) return TRUE;
	
	return FALSE;
	}

/*
 * is_live(bp, reg) - is 'reg' live at the end of block 'bp'
 *
 * Look ahead through the traversal graph to see what might happen to the
 * given register. If we can find any reachable block that references 'reg'
 * before setting it, the register is live. Scanning stops when the register
 * is set, we loop back to the starting block, or the function returns.
 */
static bool is_live(bp, reg,fhead)
	register BLOCK	*bp,*fhead;
	register int	reg;
	{
	register BLOCK	*cp;
	register INST	*ip;

	if(bp->flags & B_RET)
		return (used_rts&reg)/*?TRUE:FALSE*/;
	
	/*
	 * Clear all the mark bits
	 */
	for(cp = fhead; cp ;cp = cp->next) cp->flags &= ~B_MARK;

	bp->flags |= B_MARK;

	if(scan_ref(bp->bfall, reg)) return TRUE;

	if(scan_ref(bp->bcond, reg)) return TRUE;

	return FALSE;
	}

/*
 * bflow() - live/dead analysis for a given block
 *
 * Work backward from the end of the block, checking the status of registers.
 * To start with, figure out the state of each register as of the end of the
 * block. Then work backward, checking registers only as necessary.
 */
static void bflow(bp,fhead)
	BLOCK	*bp,*fhead;
	{
	register INST	*ip;
	register int	live = 0;
	register int	reg;

	DBG(progress())

	/*
	 * Figure out what's alive at the end of this block.
	 */
	for(reg=FIRSTREG;reg<=LASTREG;++reg) 
		{
		if(is_live(bp,RM(reg),fhead)) live|=RM(reg);
		}

	/*
	 * Now work through the instructions in the block.
	 */
	for(ip=bp->end; ip!=bp->start ;ip=ip->prev)
		{
		ip->live = live;
		for(reg=FIRSTREG; reg<=LASTREG ; ++reg)
			{
			if(ip->refs & RM(reg))      live |= RM(reg);
			else if(ip->sets & RM(reg)) live &= ~RM(reg);
			}
		}
	ip->live = live;
	}

/*
 *	livedead() - do live/dead analysis of block bh
 */
static void livedead(bh)
	register BLOCK *bh;
	{
	register BLOCK *cp;
	for(cp=bh; cp; cp=cp->next) bflow(cp, bh);
	}

/*
 *	do_health() - do live/dead analysis of sourcefile pointed by head
 */
void do_health()
	{
	register BLOCK *bh;
	labmerge();
	DBG(printf("do_health( )\010\010"))
	bh=prep();
/*	putblocks(bh); /**/
	livedead(bh);
/*	putblocks(bh); /**/
	freeblocks(bh);
	DBG(printf(") \n"))
	}
