/*.
 *	peep2.c - 2-instruction peephole optimizations
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

int p2_line;

/*.
 * ipeep2(i1) - look for 2-instruction optimizations at the given inst.
 *	returns next inst.
 */
static INST *ipeep2(i1)
	register INST	*i1;
	{
	register INST	*i2;		/* the next instruction */
	register INST	*ti2;		/* "temporary" next inst */

	register int	op1, op2;	/* opcodes, for speed */
	register int	sm1, dm1;	/* src, dst amode inst. 1 */
	register int	sr1, dr1;	/* src, dst reg. inst. 1 */
	register int	sm2, dm2;	/* src, dst amode inst. 2 */
	register int	sr2, dr2;	/* src, dst reg. inst. 2 */

	ifn(i2 = i1->next) goto next;
	if(i2->label) goto next;

	op1 = i1->opcode; sm1 = i1->src.amode; sr1 = i1->src.areg; 
	dm1 = i1->dst.amode; dr1 = i1->dst.areg;

	op2 = i2->opcode; sm2 = i2->src.amode; sr2 = i2->src.areg; 
	dm2 = i2->dst.amode; dr2 = i2->dst.areg;

	/*
	 *	move.l	#0,Dx		=>	<deleted>
	 *	move.x	Dx,X			clr.x	X
	 *
	 * Where Dx is dead. X!=Rn
	 */
	if(OP_MOVE(op1) && sm1==IMM && !i1->src.disp && dm1==REG 
	   && ISD(dr1) && (i1->flags==LENL || !i1->flags) &&
	   OP_MOVE(op2) && sm2==REG && sr2==dr1 && dm2!=REG &&
	   !(i2->live & RM(dr1)))
		{
		i2->opcode    = CLR;
		i2->src       = i2->dst;
		i2->dst.amode = NONE;
		delinst(i1);
		DBG(p2_line=__LINE__)
		goto true2;
		}

	/*
	 * Avoid "tst" instructions following instructions that
	 * set the Z flag.
	 */

	/*
	 *	move.x	X,Y		=>	move.x	X,Y
	 *	tst.x	X or Y			<deleted>
	 *
	 * Where Y is not An, because "movea" doesn't set the
	 * zero flag.
	 */
	if (OP_MOVE(op1) && op2 == TST && i1->flags == i2->flags)
		{
		/*.
		 * If pre-decrement is set on the dest. of the move,
		 * don't let that screw up the operand comparison.
		 */
		if( (opeq(&i1->dst,&i2->src) || opeq(&i1->src,&i2->src)) &&
		    ((dm1 & ~(DEC|INC)) != REG || ISD(dr1)))
			{
		    	delinst(i2);
		    	DBG(p2_line=__LINE__)
		    	goto true;
			}
		}
	/*
	 *	ARITHM.x X,Y		=>	ARITHM.x X,Y
	 *	tst.x	 Y			<deleted>
	 *
	 * Where Y is not An, because "adda" doesn't set the
	 * zero flag.
	 */
	if ((OP_SUB(op1)||OP_ADD(op1)||OP_AND(op1)||OP_OR(op1)||OP_EOR(op1)) 
	    && op2 == TST && i1->flags == i2->flags) 
		{
		/*.
		 * If pre-decrement is set on the dest. of the move,
		 * don't let that screw up the operand comparison.
		 */
		if (opeq(&i1->dst,&i2->src) &&
		    ((dm1 & ~(DEC|INC)) != REG || ISD(dr1)) )
			{
			delinst(i2);
			DBG(p2_line=__LINE__)
			goto true;
			}
		}
	/*
	 *	ext.x	Dn		=>	ext.x	Dn
	 *	tst.x	Dn			<deleted>
	 */
	if (op1 == EXT && op2 == TST && i1->flags == i2->flags) 
		{
		if (sm1 == REG && ISD(sr1) && sm2 == REG && sr1 == sr2) 
			{
		    	delinst(i2);
		    	DBG(p2_line=__LINE__)
		    	goto true;
			}
		}
	/*
	 *	move.x	X,Dn		=>	move.x	X,Dn
	 *	ext.x	Dn			<deleted>
	 *	b<cc>				b<cc>
	 *
	 *  Where Dn is dead after the "ext".
	 */
	if (OP_MOVE(op1) && op2 == EXT && i2->next &&
	    uses_cc(i2->next->opcode)) 
		{
		if (dm1 == REG && ISD(dr1) && sm2 == REG && dr1 == sr2) 
			{
			ifn (i2->live & RM(sr2))
				{
				delinst(i2);
			    	DBG(p2_line=__LINE__)
		 		goto true;
				}
			}
		}
	/*
	 *	ext.x	Dm		=>	<deleted>
	 *	tst.x	Dm			tst.y	Dm
	 *
	 * Where Dm is dead after the "tst". y = w if x = l
	 *				     y = b if x = w
	 */
	if (op1 == EXT && op2 == TST &&
	    (i1->flags & (LENL|LENW)) && !(i1->flags - i2->flags) &&
	    (sr1 == sr2) && ISD(sr1)) 
		{
		ifn (i2->live & RM(sr2))
			{
			i2->flags = (i2->flags == LENL ? LENW : LENB);
			delinst(i1);
		    	DBG(p2_line=__LINE__)
			goto true2;
			}
		}
	/*
	 *	ext.l	Dm		=>	<deleted>
	 *	INST	..N(An,Dm.l)..		INST	..N(An,Dm.w)..
	 *
	 * Where Dm is dead.
	 */
	if ((op1==EXT) && (i1->flags==LENL) && !(i2->live & RM(sr1)))
		{bool changed=FALSE;
		if(((sm2&~SYMB)==(REGIDX|XLONG)) && (sr1==i2->src.ireg))
			{
			i2->src.amode &= ~XLONG;
			changed=TRUE;
			}
		if(((dm2&~SYMB)==(REGIDX|XLONG)) && (sr1==i2->dst.ireg))
			{
			i2->dst.amode &= ~XLONG;
			changed=TRUE;
			}
		if(changed)
			{
			delinst(i1);
		    	DBG(p2_line=__LINE__)
			goto true2;
			}
		}

	/*
	 * Avoid intermediate registers.
	 */

	/*
	 *	move.x	X,Dm		=>	INST.x	X,Dn
	 *	INST.x	Dm,Dn			<deleted>
	 *
	 * Where Dm is dead, and INST is one of: add, sub, and, or, cmp.
	 */
	if (OP_MOVE(op1) && (mc40 || op1!=MOVEQ) &&
	    ((op2==ADD)||(op2==SUB)||(op2==AND)||(op2==OR)||(op2==CMP)) &&
	    (i1->flags == i2->flags) &&
	    (dm1 == REG) && ISD(dr1) && (sm2 == REG) && ISD(sr2) &&
	    (dr1 == sr2) && (dm2 == REG) && ISD(dr2)) 
		{
		ifn (i2->live & RM(sr2))
			{
			i1->opcode   = op2;
			i1->dst.areg = dr2;
			i1->live    |= RM(dr2);
			delinst(i2);
		    	DBG(p2_line=__LINE__)
			goto true;
			}
		}
	/*
	 * Avoid silly moves
	 */

	/*
	 *	move.x	X,Y		=>	move.x	X,Y
	 *	move.x	Y,X			<deleted>
	 *
	 * Y can't be an A reg.
	 */
	if (OP_MOVE(op1) && OP_MOVE(op2) && (i1->flags == i2->flags) &&
	    opeq(&i1->src, &i2->dst) && opeq(&i1->dst, &i2->src) &&
	    !(sm1 & (INC|DEC)) && !(dm1 & (INC|DEC)) && 
	    !(dm1==REG && ISA(dr1))) 
		{
	     	delinst(i2);
	    	DBG(p2_line=__LINE__)
	     	goto true;
		}
	/*
	 *	move.x	X,Y		=>	move.x	X,Rn
	 *	move.x	Y,Rn			move.x	Rn,Y
	 *
	 * Where Y isn't INC or DEC, and isn't register direct
	 * and Y doesn't depend on Rn.
	 */
	if (OP_MOVE(op1) && OP_MOVE(op2) && (dm2 == REG) && (dm1!=REG) &&
	    opeq(&i1->dst, &i2->src) && !(dm1 & (INC|DEC)) &&
	    (i1->flags == i2->flags) && !uses(i1, dr2)) 
		{
		freeop(&i1->dst);
		i1->dst   = i2->dst;
		i2->dst   = i2->src;
		i2->src   = i1->dst;
		i1->live |= RM(dr2);
		DBG(p2_line=__LINE__)
		goto true;
		}
	/*
	 *	move.l	Dm,An		=>	move.l	Dm,Ao
	 *	lea	(An),Ao			<deleted>
	 *
	 * Where An is dead.
	 */
	if (OP_MOVE(op1) && (op2 == LEA) && (sm1 == REG) && ISD(sr1) &&
	    (dm1 == REG) && ISA(dr1) && (sm2 == REGI) && (dm2 == REG) &&
	    ISA(dr2) && (dr1 == sr2)) 
		{
		ifn (i2->live & RM(sr2))
			{
			i1->dst.areg = dr2;
			delinst(i2);
		    	DBG(p2_line=__LINE__)
			goto true;
			}
		}
	/*
	 *	lea	X,An		=>	lea	X,Ao
	 *	lea	(An),Ao			<deleted>
	 *
	 * Where An is dead.
	 */
	if ((op1 == LEA) && (op2 == LEA) && (sm2 == REGI) && (dr1 == sr2)) 
		{
		ifn (i2->live & RM(sr2))
			{
			i1->dst.areg = dr2;
			delinst(i2);
		    	DBG(p2_line=__LINE__)
			goto true;
			}
		}
	/*
	 *	lea	N(Am), Am	=>	<deleted>
	 *	INST	(Am)[,...]		INST	N(Am)[,...]
	 *
	 * Where Am is either dead after the second instruction or
	 * is a direct destination of the second instruction.
	 */
	if ((op1 == LEA) && (M(sm1) == REGID) &&
	    (sr1 == dr1) && (sm2 == REGI) && (dr1 == sr2)) 
		{
		if (!(i2->live & RM(sr2)) ||
		    ((dm2 == REG) && (dr2 == sr2))) 
			{
			i2->src.amode = sm1;
			i2->src.disp = i1->src.disp;
			delinst(i1);
		    	DBG(p2_line=__LINE__)
			goto true2;
			}
		}
	/*
	 *	move.l	Am,An		=>	<deleted>
	 *	lea	[x](An[,Rx.y]),Ap	lea	[x](Am[,Rx.y]),Ap
	 *
	 * Where An is either dead after the second instruction or
	 * is a direct destination of the second instruction.
	 */
	if ( OP_MOVE(op1) && (i1->flags==LENL) && (sm1 == REG) && ISA(sr1) &&
	     (dm1 == REG) && ISA(dr1) && (op2==LEA) && (sr2 == dr1) &&
	     ((sm2 & MMASK) == REGI   ||
	      (sm2 & MMASK) == REGID  ||
	      (sm2 & MMASK) == REGIDX ) &&
	     (!(i2->live & RM(dr1)) || ((dm2 == REG) && (dr2 == dr1))) 
	    )
		{
		i2->src.areg = sr1;
		delinst(i1);
		DBG(p2_line=__LINE__)
		goto true2;
		}
	/*
	 *	move.l	Am,An		=>	<deleted>
	 *	INST	[N](An[,Rx.y]),X	INST	[N](Am[,Rx.y]),X
	 *
	 * Where An is dead. 
	 */
	if ( OP_MOVE(op1) && (i1->flags==LENL) && (sm1 == REG) && ISA(sr1) && 
	     (dm1 == REG) && ISA(dr1) && (sr2 == dr1) &&
	     ((sm2 & MMASK) == REGI   ||
	      (sm2 & MMASK) == REGID  ||
	      (sm2 & MMASK) == REGIDX ) &&
	     ((i2->live & RM(dr1)) == 0) )
		{
		i2->src.areg = sr1;
		delinst(i1);
		DBG(p2_line=__LINE__)
		goto true2;
		}
	/*
	 *					  __
	 *	s<cc>	Dn		=>	s<cc>	Dn
	 *	not.b	Dn			<deleted>
	 */
	if(scomp(op1) && op2==NOT && i2->flags==LENB && sm2==REG && 
	   ISD(sr2) && sr2==sr1)
		{
		i1->opcode=scomp(op1);
		delinst(i2);
		DBG(p2_line=__LINE__)
		goto true;
		}
	/*
	 *	move.x	#0,Rn		=>	<deleted>
	 *	cmp.x	Rn,X			tst.x	X
	 *
	 * Where Rn is dead.
	 */
	if(OP_MOVE(op1) && sm1==IMM && dm1==REG && !i1->src.disp &&
	   op2==CMP && sm2==REG && sr2==dr1 && 
	   (i1->flags?i1->flags:4)==i2->flags && !(i2->live & RM(dr1)))
		{
		i2->opcode=TST;
		i2->src=i2->dst;i2->dst.amode=NONE;
		delinst(i1);
		DBG(p2_line=__LINE__)
		goto true2;
		}
	/*
	 *	move.x	#y,Rn		=>	<deleted>
	 *	cmp.x	Rn,Rm			subq.x	#abs(y),Rm  (y>0)
	 *					addq		    (y<0)
	 *
	 * Where Rm and Rn are dead and (1 <= abs(y) <= 8).
	 */
	if(OP_MOVE(op1) && ((i1->flags?i1->flags:4)>=i2->flags) &&
	   sm1==IMM && dm1==REG && (i1->src.disp>=-8) && (i1->src.disp<=8) &&
	   op2==CMP && sm2==REG && sr2==dr1 && dm2==REG &&
	   !(i2->live & (RM(sr2)|RM(dr2))))
	   	{
	   	i2->src.amode = IMM;
		if(i1->src.disp>0) 
			{
			i2->opcode   = SUBQ;
			i2->src.disp = i1->src.disp;
			}
		else	{
			i2->opcode   = ADDQ;
			i2->src.disp = -i1->src.disp;
			}
		delinst(i1);
		DBG(p2_line=__LINE__)
		goto true2;
	   	}
next:	return ipeep2bis(i1);
true:	return i1->next;
true2:	return i2;
	}

/*.
 * peep2() - peephole optimizations with a window size of 2
 */
bool peep2()
	{
	register INST	*ip,*ni;
	register bool	changed = FALSE;

	DBG(printf("peep (2) : "))
	for(ip=head; ip ;)
		{int line=ip->lineno;
		if(ni = ipeep2(ip))
			{
			++s_peep2;
			ip = ni;
			changed = TRUE;
			DBG(printf("%d(%d) ",p2_line,line);fflush(stdout))
			}
		else	ip=ip->next;
		}
	DBG(printf("\n"); fflush(stdout))
	return changed;
	}
