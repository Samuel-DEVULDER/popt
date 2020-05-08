/*.
 *	peep2_2.c - 2-instruction peephole optimizations (following)
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

extern int p2_line;

/*.
 * ipeep2bis(i1) - look for 2-instruction optimizations at the given 
 *		   inst. Part II.
 */
INST *ipeep2bis(i1)
	register INST	*i1;
	{
	register INST	*i2;		/* the next instruction */
	register INST	*ti2;		/* "temporary" next inst */

	register int	op1, op2;	/* opcodes, for speed */
	register int	sm1, dm1;	/* src, dst amode inst. 1 */
	register int	sr1, dr1;	/* src, dst reg. inst. 1 */
	register int	sm2, dm2;	/* src, dst amode inst. 2 */
	register int	sr2, dr2;	/* src, dst reg. inst. 2 */

	ifn(i2 = i1->next) goto end3;
	if(i2->label) goto end3;

	op1 = i1->opcode; sm1 = i1->src.amode; sr1 = i1->src.areg; 
	dm1 = i1->dst.amode; dr1 = i1->dst.areg;

	op2 = i2->opcode; sm2 = i2->src.amode; sr2 = i2->src.areg; 
	dm2 = i2->dst.amode; dr2 = i2->dst.areg;

	/*
	 *	lea	N(Am),Am	=>	<deleted>
	 *	INST	X,(Am)			INST	X,N(Am)
	 *
	 * Where X doesn't reference Am, and Am is dead after the
	 * second instruction.
	 */
	if ((op1 == LEA) && (sm1 == REGID) && (sr1 == dr1) &&
	    (dm2 == REGI) && (dr1 == dr2)) 
		{
		if (!(i2->live & RM(dr2)) &&
		    ((sm2 == IMM) || (M(sm2) == ABS) || (sr2 != dr2))) 
			{
			i2->dst.amode = REGID;
			i2->dst.disp  = i1->src.disp;
			delinst(i1);
		    	DBG(p2_line=__LINE__+20000)
			goto true2;
			}
		}
	/*
	 *	lea	X,Am		=>	<deleted>
	 *	clr.x	(Am)			clr.x	X
	 *
	 * Where Am is dead.
	 */
	if ((op1 == LEA) && (op2 == CLR) && sm2 == REGI && (dr1 == sr2)) 
		{
		ifn (i2->live & RM(sr2))
			{
			i2->src = i1->src;i1->src.amode = NONE;
			delinst(i1);
		    	DBG(p2_line=__LINE__+20000)
			goto true2;
			}
		}
	/*
	 *	lea	X,Am		=>	<deleted>
	 *	move.x	Y,(Am)			move.x	Y,X
	 *
	 * Where Am is dead.
	 */
	if ((op1 == LEA) && OP_MOVE(op2) && dm2 == REGI && (dr1 == dr2)) 
		{
		ifn (i2->live & RM(dr2))
			{
			i2->dst = i1->src; i1->src.amode=NONE;
			delinst(i1);
		    	DBG(p2_line=__LINE__+20000)
			goto true2;
			}
		}
	/*
	 *	lea	X,Am		=>	<deleted>
	 *	move.x	(Am), Y			move.x	X,Y
	 *
	 * Where Am is dead.
	 */
	if ((op1 == LEA) && OP_MOVE(op2) && sm2 == REGI && (dr1 == sr2)) 
		{
		ifn (i2->live & RM(sr2))
			{
			i2->src = i1->src; i1->src.amode = NONE;
			delinst(i1);
		    	DBG(p2_line=__LINE__+20000)
			goto true2;
			}
		}
	/*
	 *	move.x	Dm,X		=>	move.x	Dm,X
	 *	cmp.x	#N,X			cmp.x	#N,Dm
	 *
	 * Where X isn't register direct.
	 *
	 * Since X generally references memory, we can compare
	 * with the register faster.
	 */
	if (OP_MOVE(op1) && OP_CMP(op2) &&
	    (i1->flags == i2->flags) && (sm2 == IMM) &&
	    (sm1 == REG) && ISD(sr1) && (dm1 != REG) &&
	    opeq(&i1->dst, &i2->dst) && !(dm1 & (INC|DEC))) 
		{
		freeop(&i2->dst);
		i2->dst.amode = REG;
		i2->dst.areg  = sr1;
		i1->live     |= RM(sr1);
	    	DBG(p2_line=__LINE__+20000)
		goto true;
		}
	/*
	 *	move.x	X,Dm		=>	move.x	X,Dm
	 *	cmp.x	#N,X			cmp.x	#N,Dm
	 *
	 * Where X isn't register direct.
	 *
	 * Since X generally references memory, we can compare
	 * with the register faster.
	 */
	if (OP_MOVE(op1) && OP_CMP(op2) &&
	    (i1->flags == i2->flags) && (sm2 == IMM) &&
	    (dm1 == REG) && ISD(dr1) && (sm1 != REG) &&
	    opeq(&i1->src, &i2->dst) && !(sm1 & (INC|DEC))) 
		{
		freeop(&i2->dst);
		i2->dst.amode = REG;
		i2->dst.areg  = dr1;
		i1->live     |= RM(sr1);
	    	DBG(p2_line=__LINE__+20000)
		goto true;
		}

	/*
	 * Try to use register indirect w/ displacement and/or index
	 */

	/*
	 *	add.l	Am,Rn		=>	<deleted>
	 *	move.l	Rn,Ao			lea	0(Am,Rn.l),Ao
	 *
	 * Where Rn is dead. UNSAFE since the first add sets the flags
	 * (if Rn=Dn). Not for 68020/30/40/60.
	 */
	if (!mc60 && !mc40 && !mc20 && 
	    (!safe || cc_modified(i2->next) || ISA(dr1)) &&
	    OP_ADD(op1) && OP_MOVE(op2) &&
	    (sm1 == REG) && ISA(sr1) &&
	    (dm2 == REG) && ISA(dr2) &&
	    (dm1 == REG) && (sm2 == REG) && (dr1 == sr2) &&
	    (i1->flags==LENL) && (i2->flags==LENL) && !(i2->live & RM(sr2)))
		{
		setupinst(i2,LEA,0,(REGIDX|XLONG),0,sr1,dr1,REG,dr2);
	    	delinst(i1);
	    	DBG(p2_line=__LINE__+20000)
	    	goto true2;
		}
	/*
	 *	add.lw	Rm,An		=>	<deleted>
	 *	...(1)				...(1)
	 *	INST.x	..[N](An)..		INST.x	..[N](An,Rm.lw)..
	 *
	 * Where An is dead and not used in (1) and Rm not set in (1). 
	 * Not for 68040/60. (not for 68020/30 if N=0).
	 */
	if(!mc60 && !mc40 && OP_ADD(op1) && sm1==REG && dm1==REG && ISA(dr1) && 
	   (i1->flags & (LENL|LENW)))
		{int m;bool flg;
		ti2 = i2;
		while (!uses(ti2, dr1)) 
			{
			if(ti2->sets & RM(sr1))	goto end0;
			if  (breakflow(ti2))	goto end0;
			ifn (ti2=ti2->next)	goto end0;
			}
		if(ti2->label) goto end0;
		if(ti2->live & RM(dr1)) goto end0;

		/*.
		 * chk disp
		 */
		if(ti2->src.amode==(REGID|SYMB) && ti2->src.areg==dr1)
			goto end0;
		if(ti2->src.amode==REGID && ti2->src.areg==dr1 && 
			!D8OK(ti2->src.disp)) goto end0;
		if(ti2->dst.amode==(REGID|SYMB) && ti2->dst.areg==dr1) 
			goto end0;
		if(ti2->dst.amode==REGID && ti2->dst.areg==dr1 && 
			!D8OK(ti2->dst.disp)) goto end0;

		flg=0;

		m=M(ti2->src.amode);
		if(last_pass && mc20 && 
			   (m==REGI || 
			   (m==REGID && ti2->src.disp==0))) goto end0;
		if((m==REGI || m==REGID) && ti2->src.areg==dr1)
			{
			ti2->src.amode = (i1->flags==LENL)?
					 (REGIDX|XLONG):REGIDX;
			if(m==REGI) ti2->src.disp=0;
			ti2->src.ireg  = sr1;
			flg=1;
			}

		m=M(ti2->dst.amode);
		if(last_pass && mc20 && 
			   (m==REGI || 
			   (m==REGID && ti2->dst.disp==0))) goto end0;
		if((m==REGI || m==REGID) && ti2->dst.areg==dr1)
			{
			ti2->dst.amode = (i1->flags==LENL)?
					 (REGIDX|XLONG):REGIDX;
			if(m==REGI) ti2->dst.disp=0;
			ti2->dst.ireg  = sr1;
			flg=1;
			}
		if(flg)	{
		    	delinst(i1);
		    	DBG(p2_line=__LINE__+20000)
		    	goto true2;
		    	}
	   	}
end0:	/*
	 *	lea	N(Am),An	=>	lea	N(Am,Ro.l),An
	 *	add.l	Ro,An			<deleted>
	 *
	 * Not for 68040. (For 68020/30 we gain 2 instructions bytes).
	 */
	if (!mc40 && (op1 == LEA) && OP_ADD(op2) &&
	    (sm1 == REGID) && (sm2 == REG) && 
	    (dm2 == REG) && ISA(dr2) &&
	    (dr1 == dr2) && D8OK(i1->src.disp)) 
		{
		i1->src.amode = REGIDX|XLONG;
		i1->src.ireg = sr2;
		delinst(i2);
	    	DBG(p2_line=__LINE__+20000)
		goto true;
		}
	/*
	 *	lea	X,Ax		=>	lea	X,Ay
	 *	move.l	Ax,Ay			<deleted>
	 *
	 * Where Ax is dead.
	 */
	if((op1 == LEA) && ISA(dr1) &&
	   OP_MOVE(op2) && (i2->flags & LENL) &&
	   (sm2 == REG) && (sr2 == dr1) &&
	   (dm2 == REG) && ISA(dr2) &&
	   !(i2->live & RM(dr1)))
	   	{
	   	i1->dst.areg=dr2;
	   	delinst(i2);
	    	DBG(p2_line=__LINE__+20000)
	    	goto true;
	   	}
	/*
	 *	move.x	An,Am		=>	<deleted>
	 *	cmp.x	X,Am			cmp.x	X,An
	 *
	 * Where Am is dead and X does not set An.
	 */
	if (OP_MOVE(op1) && OP_CMP(op2) &&
	    (i1->flags == i2->flags) &&
	    (sm1 == REG) && ISA(sr1) &&
	    (dm1 == REG) && ISA(dr1) &&
	    (dm2 == REG) && (dr2 == dr1) &&
	    !(i2->live & RM(dr1)) && !sets(i2,sr1) )
		{
		i2->dst.areg = sr1;
		delinst(i1);
	    	DBG(p2_line=__LINE__+20000)
		goto true2;
		}
	/*
	 *	move.y	Ry,Rx		=>	<deleted>
	 *	...(1)				...(1)
	 *	INST.x	..N(An,Rx.y)..		INST.x	..N(An,Ry.y)..
	 *
	 * Where Rx is dead and not used in (1) and Ry not set in (1). 
	 */
	if(OP_MOVE(op1) && (sm1==REG) && (dm1==REG))
	   	{int m;bool flg;

		ti2 = i2;
		while (!uses(ti2, dr1)) 
			{
			if (ti2->sets & RM(sr1)) goto end05;
			if  (breakflow(ti2))     goto end05;
			ifn (ti2=ti2->next)	 goto end05;
			}
		if(ti2->label) goto end05;
		if(ti2->live & RM(dr1)) goto end05;

		if(ti2->src.amode==REGIDX && ti2->src.ireg==dr1 && 
		   i1->flags!=LENW) goto end05;
		if(ti2->src.amode==(REGIDX|XLONG) && ti2->src.ireg==dr1 && 
		   i1->flags!=LENL) goto end05;
		if(ti2->dst.amode==REGIDX && ti2->dst.ireg==dr1 && 
		   i1->flags!=LENW) goto end05;
		if(ti2->dst.amode==(REGIDX|XLONG) && ti2->dst.ireg==dr1 && 
		   i1->flags!=LENL) goto end05;

		flg=0;

		m=M(ti2->src.amode);
		if(m==REGIDX && ti2->src.ireg==dr1)
			{
			ti2->src.ireg=sr1;
			flg=1;
			}

		m=M(ti2->dst.amode);
		if(m==REGIDX && ti2->dst.ireg==dr1)
			{
			ti2->dst.ireg=sr1;
			flg=1;
			}

		if(flg)	{
		    	delinst(i1);
		    	DBG(p2_line=__LINE__+20000)
		    	goto true2;
		    	}
	   	}
end05:	/*
	 *	move.l	An,Am		=>	<deleted>
	 *	...(1)				...(1)
	 *	INST.x	..N(Am[,Rx.y])..	INST.x	..N(An[,Rx.y])..
	 *
	 * Where Am is dead and not used in (1) and An not set in (1). 
	 */
	if(OP_MOVE(op1) && (sm1==REG) && ISA(sr1) && (dm1==REG) && ISA(dr1)&&
	   i1->flags==LENL)
	   	{int m;bool flg;

		ti2 = i2;
		while (!uses(ti2, dr1)) 
			{
			if (ti2->sets & RM(sr1)) goto end1;
			if  (breakflow(ti2))     goto end1;
			ifn (ti2=ti2->next)	 goto end1;
			}
		if(ti2->label) goto end1;
		if(ti2->live & RM(dr1)) goto end1;

		flg=0;

		m=M(ti2->src.amode);
		if((m==REGI || m==REGID || m==REGIDX) && ti2->src.areg==dr1)
			{
			ti2->src.areg=sr1;
			flg=1;
			}

		m=M(ti2->dst.amode);
		if((m==REGI || m==REGID || m==REGIDX) && ti2->dst.areg==dr1)
			{
			ti2->dst.areg=sr1;
			flg=1;
			}

		if(flg)	{
		    	delinst(i1);
		    	DBG(p2_line=__LINE__+20000)
		    	goto true2;
		    	}
	   	}
end1:	/*
	 *	addq	#N,sp		=>	addq	#N-4,sp 
	 *	....				....
	 *	<stuff that  >			<stuff that  >
	 *	<doesn't use >			<doesn't use >
	 *	<SP ...      >			<SP ...      >
	 *	....				....
	 *	INST.l	..-(sp)..	=>	INST.l	..(sp)..
	 *	
	 * addq or lea N(SP),SP. addq is deleted if N==4.
	 */
	if(((OP_ADD(op1) && sm1==IMM) || 
	    (op1==LEA && sm1==REGID && sr1==SP)) && 
	   i1->src.disp>=4 && dm1==REG && dr1==SP) 
		{
		bool ok;
		ti2 = i2;
		while (ti2->opcode==BSR||ti2->opcode==JSR||!uses(ti2, SP)) 
			{
			ifn (ti2->next)	     goto end2;
			if  (breakflow(ti2)) goto end2;
			ti2 = ti2->next;
			}
		if (ti2->label) goto end2;

		ok = FALSE;
		if (ti2->opcode==PEA && ti2->src.amode==REGI)
			{
			setupinst(ti2,MOVE,LENL,REG,ti2->src.areg,REGI,SP);
			ok = TRUE;
			}
		else if (ti2->flags==LENL && 
			 ti2->dst.amode==(REGI|DEC) && ti2->dst.areg==SP) 
			{
		    	ti2->dst.amode = REGI;
			ok = TRUE;
			}
		else if (ti2->flags==LENL && 
			 ti2->src.amode==(REGI|DEC) && ti2->src.areg==SP) 
			{
		    	ti2->src.amode = REGI;
			ok = TRUE;
			}
		if(ok)	{
			ifn(i1->src.disp -= 4) delinst(i1);
		    	DBG(p2_line=__LINE__+20000)
		    	goto true2;
			}
		}
end2:	/*
	 *	addq	#N,sp		=>	addq	#N-2,sp
	 *	....				....
	 *	<stuff that  >			<stuff that  >
	 *	<doesn't use >			<doesn't use >
	 *	<SP ...      >			<SP ...      >
	 *	....				....
	 *	INST.w	..-(sp)..	=>	INST.w	..(sp)..
	 *
	 * addq or lea N(SP),SP. addq is deleted if N==2.
	 */
	if(((OP_ADD(op1) && sm1==IMM) || 
	    (op1==LEA && sm1==REGID && sr1==SP)) && 
	   i1->src.disp>=2 && dm1==REG && dr1==SP) 
		{
		bool ok;

		ti2 = i2;
		while (ti2->opcode==BSR||ti2->opcode==JSR||!uses(ti2, SP)) 
			{
			ifn (ti2->next)	     goto end3;
			if  (breakflow(ti2)) goto end3;
			ti2 = ti2->next;
			}
		if(ti2->label) goto end3;
		
		ok = FALSE;
		if(ti2->flags == LENW &&
		   ti2->dst.amode == (REGI|DEC) && ti2->dst.areg == SP) 
			{
		    	ti2->dst.amode = REGI;
		    	ok = TRUE;
		    	}
		else if(ti2->flags == LENW &&
		   	ti2->src.amode == (REGI|DEC) && ti2->src.areg == SP) 
			{
		    	ti2->src.amode = REGI;
		    	ok = TRUE;
		    	}

		if(ok)	{
			ifn(i1->src.disp -= 2) delinst(i1);
		    	DBG(p2_line=__LINE__+20000)
		    	goto true2;
			}
		}
end3:	return ipeep2ter(i1);
true:	return i1->next;
true2:	return i2;
	}
