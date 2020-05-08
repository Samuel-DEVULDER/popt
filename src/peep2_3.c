/*.
 *	peep2_3.c - 2-instruction peephole optimizations (following)
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

extern int p2_line;

/*.
 * ipeep2ter(i1) - look for 2-instruction optimizations at the given 
 *		   inst. Part III.
 */
INST *ipeep2ter(i1)
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
	 *	lea    N(An),Am		=>	<deleted>
	 *	...(1)				...(1)
	 *	INST.x ..[M](Am[,Rx.y])..	INST.x ..[N+]M(An[,Rx.y])..
	 *
	 * Where Am is dead and not used in (1) and An not set in (1). 
	 * If Rx==Am then use 2*N instead of N.
	 */
	if(op1==LEA && sm1==REGID)
	   	{int m;bool flg;
		ti2 = i2;
		while (!uses(ti2, dr1)) 
			{
			if (ti2->sets & RM(sr1)) goto end4;
			if  (breakflow(ti2))     goto end4;
			ifn (ti2=ti2->next)	 goto end4;
			}
		if(ti2->label) goto end4;
		if(ti2->live & RM(dr1)) goto end4;
		
		/*.
		 * chk disp 
		 */
		m=ti2->src.amode&~XLONG;
		if((m==REGI || m==REGID || m==REGIDX) && ti2->src.areg==dr1)
			{
			if(m==REGI)	m = i1->src.disp;
			else if(m==REGIDX)
				{
				if(ti2->src.ireg==dr1)
					m = ti2->src.disp+2*i1->src.disp;
				else	m = ti2->src.disp+i1->src.disp;
				}
			else		m = ti2->src.disp+i1->src.disp;
			ifn(D8OK(m)) goto end4;
			}
		m=ti2->dst.amode&~XLONG;
		if((m==REGI || m==REGID || m==REGIDX) && ti2->dst.areg==dr1)
			{
			if(m==REGI)	m = i1->src.disp;
			else if(m==REGIDX)
				{
				if(ti2->src.ireg==dr1)
					m = ti2->dst.disp+2*i1->src.disp;
				else	m = ti2->dst.disp+i1->src.disp;
				}
			else		m = ti2->dst.disp+i1->src.disp;
			ifn(D8OK(m)) goto end4;
			}

		flg=0;

		m=ti2->src.amode&~XLONG;
		if((m==REGI || m==REGID || m==REGIDX) && ti2->src.areg==dr1)
			{
			if(m==REGI)
				{
				ti2->src.amode=REGID;
				ti2->src.disp=i1->src.disp;
				}
			else if(m==REGIDX)
				{
				if(ti2->src.ireg==dr1)
					{
					ti2->src.ireg=sr1;
					ti2->src.disp+=2*i1->src.disp;
					}
				else	ti2->src.disp+=i1->src.disp;
				}
			else	ti2->src.disp+=i1->src.disp;
			ti2->src.areg=sr1;
			flg=1;
			}

		m=ti2->dst.amode&~XLONG;
		if((m==REGI || m==REGID || m==REGIDX) && ti2->dst.areg==dr1)
			{
			if(m==REGI)
				{
				ti2->dst.amode=REGID;
				ti2->dst.disp=i1->src.disp;
				}
			else if(m==REGIDX)
				{
				if(ti2->dst.ireg==dr1)
					{
					ti2->dst.ireg=sr1;
					ti2->dst.disp+=2*i1->src.disp;
					}
				else	ti2->dst.disp+=i1->src.disp;
				}
			else	ti2->dst.disp+=i1->src.disp;
			ti2->dst.areg=sr1;
			flg=1;
			}
		if(flg)	{
		    	delinst(i1);
		    	DBG(p2_line=__LINE__+30000)
		    	goto true2;
		    	}
	   	}
end4:	/*
	 *	lea	N(An,Rx.y),Am	=>	<deleted>
	 *	...(1)				...(1)
	 *	INST.x	..[M](Am)..		INST.x	..N[+M](An,Rx.y)..
	 *
	 * Where Am is dead and not used in (1) and (An,Dx) are not set 
	 * in (1). 
	 */
	if(op1==LEA && (sm1&~XLONG)==REGIDX)
	   	{int m;bool flg;
		ti2 = i2;
		m=RM(i1->src.ireg);
		while (!uses(ti2, dr1)) 
			{
			if (ti2->sets & (RM(sr1)|m)) goto end5;
			if (breakflow(ti2)) goto end5;
			ifn (ti2=ti2->next) goto end5;
			}
		if(ti2->label) goto end5;
		if(ti2->live & RM(dr1)) goto end5;

		/*.
		 * chk disp
		 */
		m=ti2->src.amode&~XLONG;
		if((m==REGI || m==REGID || m==REGIDX) && ti2->src.areg==dr1)
			{
			if(m==REGI)	m = i1->src.disp;
			else		m = ti2->src.disp+i1->src.disp;
			ifn(D8OK(m)) goto end5;
			}
		m=ti2->dst.amode&~XLONG;
		if((m==REGI || m==REGID || m==REGIDX) && ti2->dst.areg==dr1)
			{
			if(m==REGI)	m = i1->src.disp;
			else		m = ti2->dst.disp+i1->src.disp;
			ifn(D8OK(m)) goto end5;
			}

		flg=0;

		m=ti2->src.amode&~XLONG;
		if((m==REGI || m==REGID) && ti2->src.areg==dr1)
			{
			if(m==REGI)	ti2->src.disp=i1->src.disp;
			else		ti2->src.disp+=i1->src.disp;
			ti2->src.amode=i1->src.amode;
			ti2->src.areg=sr1;
			ti2->src.ireg=i1->src.ireg;
			flg=1;
			}

		m=ti2->dst.amode&~XLONG;
		if((m==REGI || m==REGID) && ti2->dst.areg==dr1)
			{
			if(m==REGI)	ti2->dst.disp=i1->src.disp;
			else		ti2->dst.disp+=i1->src.disp;
			ti2->dst.amode=i1->src.amode;
			ti2->dst.areg=sr1;
			ti2->dst.ireg=i1->src.ireg;
			flg=1;
			}
		if(flg)	{
		    	delinst(i1);
		    	DBG(p2_line=__LINE__+30000)
		    	goto true2;
		    	}
	   	}
end5:	/*
	 *	move.?	#n,Rn		=>	move.?	#n,Rn
	 *	....				....
	 *	<stuff>				<stuff>
	 *	....				....
	 *	move.?	#n,Rn		=>	<deleted>
	 *
	 * Where <stuff> doesn't set Rn. Also make sure that
	 * the second move isn't followed by a conditional branch.
	 * In that case leave everything alone since the branch
	 * probably relies on flags set by the move.
	 * UNSAFE since last instruction deleted and may set flags.
	 */
	if (OP_MOVE(op1) && (sm1 == IMM) && (dm1 == REG)) 
		{
		for(ti2 = i2;
		    (ti2 && !breakflow(ti2) && !sets(ti2, dr1));
		    ti2 = ti2->next);

		ifn (ti2) goto end13;

		if(OP_MOVE(ti2->opcode) && ti2->src.amode==IMM && 
		   ti2->src.disp==i1->src.disp && ti2->dst.amode==REG &&
		   ti2->dst.areg==dr1 &&
		   (!safe || cc_modified(ti2->next)) && 
		   !(ti2->next && uses_cc(ti2->next->opcode)))
			{
			delinst(ti2);
			DBG(p2_line=__LINE__+30000)
			goto true;
			}
		}
end13:	/*
	 *	move.?	Rm,Rn		=>	move.?	Rm,Rn
	 *	....				....
	 *	<stuff>				<stuff>
	 *	....				....
	 *	move.?	Rm,Rn		=>	<deleted>
	 *
	 * Where <stuff> doesn't set Rm or Rn. Also make sure that
	 * the second move isn't followed by a conditional branch.
	 * In that case leave everything alone since the branch
	 * probably relies on flags set by the move.
	 * UNSAFE since last instruction deleted and may set flags.
	 */
	if (OP_MOVE(op1) && (sm1 == REG) && (dm1 == REG)) 
		{
		int s1   = sr1;	/* source reg of inst. 1 */
		bool flg = FALSE;
		ti2      = i2;
		while (ti2 && !sets(ti2, s1) && (flg = !sets(ti2, dr1)) && 
		       !breakflow(ti2)) ti2 = ti2->next;

		ifn (ti2) flg = FALSE;

		if(flg && OP_MOVE(ti2->opcode) && (i1->flags==ti2->flags) &&
			    (ti2->src.amode==REG) && (ti2->dst.amode==REG) &&
			    (sr1 == ti2->src.areg) &&
			    (dr1 == ti2->dst.areg) &&
			    (!ti2->next || !uses_cc(ti2->next->opcode)) &&
			    (!safe || cc_modified(ti2->next)) ) 
			{
			delinst(ti2);
			DBG(p2_line=__LINE__+30000)
			goto true;
			}
		}
	/*
	 *	move.l	Am,Dn		=>	move.l	Am,Ao
	 *	....				....
	 *	<stuff>				<stuff>
	 *	....				....
	 *	move.l	Dn,Ao		=>	<deleted>
	 *
	 * Where "stuff" doesn't set Dn. 
	 * UNSAFE since the first move may sets the flags.
	 */
	if (OP_MOVE(op1) && (i1->flags & LENL) &&
	    (sm1 == REG) && ISA(sr1) &&
	    (dm1 == REG) && ISD(dr1)) 
		{
		ti2 = i2;
		while (!sets(ti2, dr1)) 
			{
			if (breakflow(ti2)) goto end14;
			if (OP_MOVE(ti2->opcode) && (ti2->flags & LENL) &&
			    (ti2->src.amode == REG) && ISD(ti2->src.areg) &&
			    (ti2->dst.amode == REG) && ISA(ti2->dst.areg) &&
			    (dr1 == ti2->src.areg)) 
				{
				/*.
				 * If the intermediate register isn't dead,
				 * then we have to keep using it.
				 */
				if(ti2->live & RM(ti2->src.areg)) goto end14;
				if(safe &&!cc_modified(ti2->next))goto end14;
				i1->dst.areg = ti2->dst.areg;
			     	delinst(ti2);
			    	DBG(p2_line=__LINE__+30000)
			     	goto true;
				}
			ifn (ti2->next) goto end14;
			ti2 = ti2->next;
			}
		}
end14:	/*
	 * Try to use the pre-decrement an post modes whenever possible.
	 */

	/*
	 *	sub[q].lw #1,Am		=>	<deleted>
	 *	....				....
	 *	<stuff>				<stuff>
	 *	....				....
	 *	INST.b	..(Am)..	=>	INST.b	..-(Am)..
	 *
	 * Nothing in "stuff" can refer to Am.
	 */
	if (OP_SUB(op1) && (i1->flags & (LENL|LENW)) &&
	    (sm1 == IMM) && (i1->src.disp == 1) &&
	    (dm1 == REG) && ISA(dr1)) 
		{INST *ti2=i2;
		while (ti2) 
			{
			if (breakflow(ti2)) goto end24;
			if (ti2->src.amode == REGI && ti2->src.areg == dr1) 
				{
				ifn (ti2->flags==LENB) goto end24;
				ti2->src.amode |= DEC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+30000)
			    	goto true2;
				}
			if (ti2->dst.amode == REGI && ti2->dst.areg == dr1) 
				{
				ifn (ti2->flags==LENB) goto end24;
				ti2->dst.amode |= DEC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+30000)
			    	goto true2;
				}
			if (uses(ti2, dr1))	goto end24;
			ifn (ti2->next)		goto end24;
			else			ti2 = ti2->next;
			}
		}
end24:	/*
	 *	sub[q].lw #2,Am		=>	<deleted>
	 *	....				....
	 *	<stuff>				<stuff>
	 *	....				....
	 *	INST.w	..(Am)..	=>	INST.w	..-(Am)..
	 *
	 * Nothing in "stuff" can refer to Am.
	 */
	if (OP_SUB(op1) && (i1->flags & (LENL|LENW)) &&
	    (sm1 == IMM) && (i1->src.disp == 2) &&
	    (dm1 == REG) && ISA(dr1)) 
		{INST *ti2=i2;
		while (ti2) 
			{
			if (breakflow(ti2)) goto end26;
			if (ti2->src.amode == REGI && ti2->src.areg == dr1) 
				{
				ifn (ti2->flags==LENW)	goto end26;
				ti2->src.amode |= DEC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+30000)
			    	goto true2;
				}
			if (ti2->dst.amode == REGI && ti2->dst.areg == dr1) 
				{
				ifn (ti2->flags==LENW) goto end26;
				ti2->dst.amode |= DEC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+30000)
			    	goto true2;
				}
			if (uses(ti2, dr1)) goto end26;
			ifn (ti2->next)     goto end26;
			else		    ti2 = ti2->next;
			}
		}
end26:	/*
	 *	sub[q].lw #4,Am		=>	<deleted>
	 *	....				....
	 *	<stuff>				<stuff>
	 *	....				....
	 *	INST.l	..(Am)..	=>	INST.l	..-(Am)..
	 *
	 * Nothing in "stuff" can refer to Am.
	 */
	if (OP_SUB(op1) && (i1->flags & (LENL|LENW)) &&
	    (sm1 == IMM) && (i1->src.disp == 4) &&
	    (dm1 == REG) && ISA(dr1)) 
		{INST *ti2=i2;
		while (ti2)
			{
			if (breakflow(ti2)) goto end28;
			if (ti2->src.amode == REGI && ti2->src.areg == dr1) 
				{
				ifn (ti2->flags==LENL)	goto end28;
				ti2->src.amode |= DEC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+30000)
			    	goto true2;
				}
			if (ti2->dst.amode == REGI && ti2->dst.areg == dr1) 
				{
				ifn (ti2->flags==LENL) goto end28;
				ti2->dst.amode |= DEC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+30000)
			    	goto true2;
				}
			if (uses(ti2, dr1)) goto end28;
			ifn (ti2->next)	    goto end28;
			else		    ti2 = ti2->next;
			}
		}
end28:
next:	return ipeep2_4(i1);
true:	return i1->next;
true2:	return i2;
	}
