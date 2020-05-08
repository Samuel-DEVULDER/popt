/*.
 *	jsropt.c - Optimizations of jsr/bsr statements.
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

#define	IS_CALL(op,md)	((op == BSR || op == JSR) && md == (ABS|SYMB))
#define	IS_JUMP(op,md)	((op == BRA || op == JMP) && md == (ABS|SYMB))

static int j_line;
static bool jump_table;

static INST *ipeep(i1)
	INST *i1;
	{
	int op1,sm1,dm1,sr1,dr1;
	INST *i2;
	int op2,sm2,dm2,sr2,dr2;
	
	op1 = i1->opcode;
	sm1 = i1->src.amode;
	sr1 = i1->src.areg;
	dm1 = i1->dst.amode;
	dr1 = i1->dst.areg;
	
	if(i2 = i1->next)
		{
		op2 = i2->opcode;
		sm2 = i2->src.amode;
		sr2 = i2->src.areg;
		dm2 = i2->dst.amode;
		dr2 = i2->dst.areg;
		}
	else	{
		op2 = UNKWN;
		sm2 = NONE;
		dm2 = NONE;
		}

	/*
	 *	jsr	LBL(pc)		=>	bsr	LBL
	 */
	if(op1==JSR)
		{
		if(sm1==(PCD|SYMB))
			{
			i1->opcode    = BSR;
			i1->src.amode = ABS|SYMB;
			DBG(j_line=__LINE__)
			goto true;
			}
/*		else if(sm1==PCD)
			{	char s[12];
			i1->opcode    = BSR;
			i1->src.amode = ABS|SYMB;
			if(i1->src.disp>=0) sprintf(s,"*+%d",i1->src.disp);
			else		    sprintf(s,"*-%d",-i1->src.disp);
			i1->src.astr  = strsave(s);
			DBG(j_line=__LINE__)
			goto true;
			}
*/		}
	/*
	 *	jmp	LBL(pc)		=>	bra	LBL
	 *
	 * Not for jump tables. (bra might become bra.s, thus reduction of
	 * 2 bytes, thus mismatching address (jmp LBL(pc) takes always 4 
	 * bytes)).
	 */
	if(op1==JMP && !jump_table)
		{
		if(sm1==(PCD|SYMB))
			{
			i1->opcode    = BRA;
			i1->src.amode = ABS|SYMB;
			DBG(j_line=__LINE__)
			goto true;
			}
/*		else if(sm1==PCD)
			{	char s[12];
			i1->opcode    = BRA;
			i1->src.amode = ABS|SYMB;
			if(i1->src.disp>=0) sprintf(s,"*+%d",i1->src.disp);
			else		    sprintf(s,"*-%d",-i1->src.disp);
			i1->src.astr  = strsave(s);
			DBG(j_line=__LINE__;)
			goto true;
			}
*/		}
	/*
	 *	jsr	XXX		=>	jmp	XXX
	 *	rts				<deleted>
	 *
	 * The rts line must not bear a label.
	 */
	if(op1 == JSR && op2 == RTS && !i2->label) 
		{
		i1->opcode = JMP;
	    	delinst(i2);
	    	DBG(j_line=__LINE__)
	    	goto true;		
		}
	/*
	 *	bsr	XXX		=>	bra	XXX
	 *	rts				<deleted>
	 *
	 * The rts line must not bear a label.
	 */
	if(op1 == BSR && op2 == RTS && !i2->label)
		{
		i1->opcode = BRA;
	    	delinst(i2);
	    	DBG(j_line=__LINE__)
	    	goto true;		
		}
	/*
	 *	bsr	lbl1		=>	pea	lbl2
	 *	bra	lbl2			bra	lbl1
	 *	
	 * Inst jsr or bsr. bra or jmp. We need not test is bra bears
	 * a label since braopt has been performed before. 
	 * Not for 68020/30.
	 */
	if(!mc20 && IS_CALL(op1,sm1) && IS_JUMP(op2,sm2))
		{char *lbl1=i1->src.astr;
		i1->opcode    = PEA;
		i1->src.astr  = i2->src.astr;
		i1->src.amode = ABS|SYMB;
		i2->opcode    = (op1==JSR)?JMP:BRA;
		i2->src.astr  = lbl1;
		i2->src.amode = ABS|SYMB;
	    	DBG(j_line=__LINE__)
	    	goto true;		
		}
	return NULL;
true:	return i1->next;
	}

/*.
 * peepjsr() - peephole optimizations for jsr/jmp/bsr/bra
 */
void peepjsr()
	{
	register INST	*ip,*ni;
	register bool	changed;

	jump_table = FALSE;
	DBG(printf("peep jsr : "))
	do
		{
		changed = FALSE;
		for(ip=head; ip ;)
			{int line=ip->lineno; int m;
			/*.
			 *	hack to detect table of jumps.
			 */
			if(ip->label && (*ip->label=='_' || *ip->label=='@')) 
				jump_table = FALSE;
			m = M(ip->src.amode);
			if(ip->opcode == JMP && 
			   (m==PCDX || m==REGI || m==REGID || m==REGIDX))
			   	{
			   	jump_table = TRUE;
			   	DBG(printf("<%d> ",ip->lineno);fflush(stdout);)
			   	}
			if(ni = ipeep(ip))
				{
				++s_jsr;
				ip = ni;
				changed = TRUE;
				DBG(printf("%d(%d) ",j_line,line);fflush(stdout))
				}
			else	ip=ip->next;
			}
		}
	while(changed);
	DBG(printf("\n"); fflush(stdout))
	}
