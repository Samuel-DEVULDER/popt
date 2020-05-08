/*
 *	mc40opt.c - some optimisations for 68040.
 *
 *	Suggestions by Pascal Lauly (lauly@cnam.fr). Thanks Pascal.
 */
/*.	(c) by Samuel DEVULDER
 */

#include "popt.h"

static int m4_line;

int do_arithm(a,op,b)
	int a,op,b;
	{
	if(OP_ADD(op)) return a+b;
	if(OP_SUB(op)) return a-b;
	if(OP_AND(op)) return a&b;
	if(OP_OR (op)) return a|b;
	if(OP_EOR(op)) return a^b;
	if(op==LSL)    return ((unsigned)a)<<b;
	if(op==LSR)    return ((unsigned)a)>>b;
	if(op==LSL)    return a<<b;
	if(op==LSR)    return a>>b;
	}

/*.
 *	is_mem_opnd() - is operand in memory ?
 */
static bool is_mem_opnd(op)
	struct opnd *op;
	{
	switch(M(op->amode))
		{
		case ABS: case REGI: case REGID: case REGIDX: case PCD:
		case PCDX:
		return TRUE;

		default: return FALSE;
		}
	}

/*.
 *	is_no_mem_opnd() - is operand not in memory ?
 */
static bool is_no_mem_opnd(op)
	struct opnd *op;
	{
	switch(M(op->amode))
		{
		case NONE: case REG: case IMM: case MSK:
		return TRUE;

		default: return FALSE;
		}
	}

/*.
 *	is_mem_inst() - is instruction referencing memory ?
 */
static bool is_mem_inst(ip)
	INST *ip;
	{
	if(!ip || !is_real_inst(ip->opcode) || ip->opcode==LEA)
		return FALSE;
	return is_mem_opnd(&ip->src)||is_mem_opnd(&ip->dst);
	}

/*.
 *	is_no_mem_inst() - is instruction not referencing memory ?
 */
static bool is_no_mem_inst(ip)
	INST *ip;
	{
	if(!ip || !is_real_inst(ip->opcode) || ip->opcode==LEA)
		return FALSE;
	return is_no_mem_opnd(&ip->src)&&is_no_mem_opnd(&ip->dst);
	}

/*.
 *	ipeep(i1) - performe optimizations for 68040 for instructions
 *	starting at i1.
 */
static INST *ipeep(i1)
	register INST	*i1;
	{
	int op1,sm1,dm1,sr1,dr1,fl;
	INST *i2,*i3;
	int op2,sm2,dm2,sr2,dr2;
	int op3,sm3,dm3,sr3,dr3;

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
		if(i3 = i2->next)
			{
			op3 = i3->opcode;
			sm3 = i3->src.amode;
			sr3 = i3->src.areg;
			dm3 = i3->dst.amode;
			dr3 = i3->dst.areg;
			}
		else	{
			op3 = UNKWN;
			sm3 = NONE;
			dm3 = NONE;
			}
		}
	else	{
		op3 = op2 = UNKWN;
		sm3 = sm2 = NONE;
		dm3 = dm2 = NONE;
		}

	/*.
	 *	OPTIMIZATIONS FOR 68040:
	 */

	/*
	 *	ext.w	Dn		=>	extb.l	Dn
	 *	ext.l	Dn			<deleted>
	 *
	 * Only if newinsts is set.
	 */
	if(newinsts && op1==op2 && op1==EXT && i1->flags==LENW && i2->flags==LENL)
		{
		i1->opcode = EXTB;
		i1->flags  = LENL;
		delinst(i2);
		DBG(m4_line=__LINE__);
		goto true;
		}
	/*
	 *	sub.l	Ax,Ax		=>	lea	0.W,Ax
	 */
	if(OP_SUB(op1) && sm1==REG && sm2==REG && ISA(sr1) && sr1==sr2)
		{
		setupinst(i1,LEA,0,ABS|XWORD,0,REG,sr1);
		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	swap	dx		=>	and.l	#65535,dx
	 *	clr.w	dx			<deleted>
	 *	swap	dx			<deleted>
	 */
	if(op1==SWAP && op2==CLR && op3==SWAP && (i2->flags&LENW) &&
	   sm1==REG && ISD(sr1) && sm2==REG && sr2==sr1 &&
	   sm3==REG && sr3==sr1 && !i2->label && !i3->label)
		{
		setupinst(i1,AND,LENL,IMM,65535,REG,sr1);
		delinst(i2);
		delinst(i3);
		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	asl.l	#N,Dn		=>	<deleted>
	 *	...(1)				...(1)
	 *	INST	..(Ap,Dn.L)..		INST	..(Ap,Dn.L*M)..
	 *
	 * M = 2^N.
	 * Where (1) does not use Dn and Dn is dead after last instruction.
	 */
	if(last_pass && newinsts &&
	   (op1==ASL || op1==LSL) && sm1==IMM && dm1==REG && ISD(dr1) && 
	   i1->src.disp>=1 && i1->src.disp<=3 && i1->flags==LENL)
	   	{bool ok=FALSE;
		INST *ti2 = i2;
		while (!uses(ti2, dr1)) 
			{
			if  (breakflow(ti2))     goto end1;
			ifn (ti2=ti2->next)	 goto end1;
			}
		if(ti2->label)		goto end1;
		if(ti2->live & RM(dr1)) goto end1;

		if((ti2->src.amode&~SYMB)==(REGIDX|XLONG) && 
		   ti2->src.ireg == dr1) 
			{
			if((ti2->dst.amode&~SYMB)==REGIDX &&
			   ti2->dst.ireg == dr1) goto end1;
			}
		if((ti2->dst.amode&~SYMB)==(REGIDX|XLONG) && 
		   ti2->dst.ireg == dr1) 
			{
			if((ti2->src.amode&~SYMB)==REGIDX &&
			   ti2->src.ireg == dr1) goto end1;
			}

		if((ti2->src.amode&~SYMB)==(REGIDX|XLONG) && 
		   ti2->src.ireg == dr1) 
			{
			if     (i1->src.disp == 1) ti2->src.amode |= X2;
			else if(i1->src.disp == 2) ti2->src.amode |= X4;
			else if(i1->src.disp == 3) ti2->src.amode |= X8;
			ok = TRUE;
			}
		if((ti2->dst.amode&~SYMB)==(REGIDX|XLONG) && 
		   ti2->dst.ireg == dr1) 
			{
			if     (i1->src.disp == 1) ti2->dst.amode |= X2;
			else if(i1->src.disp == 2) ti2->dst.amode |= X4;
			else if(i1->src.disp == 3) ti2->dst.amode |= X8;
			ok = TRUE;
			}
		if(ok)
			{
			delinst(i1);
			DBG(m4_line=__LINE__)
			goto true2;
			}
	   	}
	/*
	 *	add.l	Dn,Dn		=>	<deleted>
	 *	...(1)				...(1)
	 *	INST	..(Ap,Dn.L)..		INST	..(Ap,Dn.L*2)..
	 *
	 * Where (1) does not use Dn and Dn is dead after last instruction.
	 */
	if(last_pass && newinsts &&
	   (op1==ADD) && sm1==REG && dm1==REG && ISD(dr1) && sr1==dr1 &&
	   i1->flags==LENL)
	   	{bool ok=FALSE;
		INST *ti2 = i2;
		while (!uses(ti2, dr1)) 
			{
			if  (breakflow(ti2))     goto end1;
			ifn (ti2=ti2->next)	 goto end1;
			}
		if(ti2->label)		goto end1;
		if(ti2->live & RM(dr1)) goto end1;

		if((ti2->src.amode&~SYMB)==(REGIDX|XLONG) && 
		   ti2->src.ireg == dr1) 
			{
			if((ti2->dst.amode&~SYMB)==REGIDX &&
			   ti2->dst.ireg == dr1) goto end1;
			}
		if((ti2->dst.amode&~SYMB)==(REGIDX|XLONG) && 
		   ti2->dst.ireg == dr1) 
			{
			if((ti2->src.amode&~SYMB)==REGIDX &&
			   ti2->src.ireg == dr1) goto end1;
			}

		if((ti2->src.amode&~SYMB)==(REGIDX|XLONG) && 
		   ti2->src.ireg == dr1) 
			{
			ti2->src.amode |= X2;
			ok = TRUE;
			}
		if((ti2->dst.amode&~SYMB)==(REGIDX|XLONG) && 
		   ti2->dst.ireg == dr1) 
			{
			ti2->dst.amode |= X2;
			ok = TRUE;
			}
		if(ok)
			{
			delinst(i1);
			DBG(m4_line=__LINE__)
			goto true2;
			}
	   	}
end1:	/*
	 *	lea	n(Ax),Ax        =>      adda.w  #n,Ax
	 */
	if(op1==LEA && sm1==REGID && sr1==dr1)
		{
		setupinst(i1,ADDA,LENW,IMM,i1->src.disp,REG,sr1);
		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	lea	0(Am,Rn.l),Ao   =>      move.l  Am, Ao
	 *					add.l	Rn, Ao
	 */
	if (op1==LEA && sm1==(REGIDX|XLONG) && i1->src.disp==0)
		{int Rn=i1->src.ireg;
		setupinst(i1,           MOVE,LENL,REG,sr1,REG,dr1);
		setupinst(insinst(i1),  ADD,LENL,REG,Rn,REG,dr1);
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	INST	0(Am,Rn.l),X	=>	add.l   Rn,Am
	 *					INST	(Am),X
	 *
	 * Where Am is dead. X must not refer Am.
	 */
	if (sm1==(REGIDX|XLONG) && i1->src.disp==0 &&
	    !(i1->live & RM(sr1)) )
		{int Rn=i1->src.ireg;
		int m=M(i1->dst.amode);
		ifn((m==REGI || m==REGID || m==REGIDX) && dr1==sr1)
			{INST *ti=insinst(i1);
			ti->opcode    = i1->opcode;
			ti->flags     = i1->flags;
			ti->src.amode = REGI;
			ti->src.areg  = sr1;
			ti->dst       = i1->dst;
			setupinst(i1, ADD,LENL,REG,Rn,REG,sr1);
			DBG(m4_line=__LINE__)
			goto true2;
			}
		}
	/*
	 *	INST	X,0(Am,Rn.l)	=>	add.l   Rn,Am
	 *					INST	X,(Am)
	 *
	 * Where Am is dead. X must not refer Am.
	 */
	if (dm1==(REGIDX|XLONG) && i1->dst.disp==0 &&
	    !(i1->live & RM(dr1)) )
		{int Rn=i1->dst.ireg;
		int m=M(i1->src.amode);
		ifn((m==REGI || m==REGID || m==REGIDX) && dr1==sr1)
			{INST *ti=insinst(i1);
			ti->opcode    = i1->opcode;
			ti->flags     = i1->flags;
			ti->src       = i1->src;
			ti->dst.amode = REGI;
			ti->dst.areg  = dr1;
			setupinst(i1, ADD,LENL,REG,Rn,REG,dr1);
			DBG(m4_line=__LINE__)
			goto true2;
			}
		}
	/*
	 *	lea	N(Am,Rn.l),Ao   =>      LEA     N(Am),Ao
	 *					add.l	Rn,Ao
	 */
	if (op1==LEA && (sm1&~SYMB)==(REGIDX|XLONG))
		{int Rn=i1->src.ireg;
		i1->src.amode = REGID;
		setupinst(insinst(i1),ADD,LENL,REG,Rn,REG,dr1);
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	bclr.l	#b,dx		=>	and.l	#n,dx
	 *
	 * n = ~(1<<b) (not for size opt.)
	 */
	if(!size && op1==BCLR && sm1==IMM && dm1==REG && ISD(dr1))
		{
		setupinst(i1,AND,LENL,IMM,~(1<<i1->src.disp),REG,dr1);
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	st	X		=>	move.b	#-1,X
	 */
	if(op1==ST)
		{
		i1->opcode	= MOVE;
		i1->flags	= LENB;
		i1->dst 	= i1->src;
		i1->src.amode	= IMM;
		i1->src.disp	= -1;
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	move.l	#X,dx		=>	move.l	#Y,dx
	 *	ARITM	#n,dx			<deleted>
	 *
	 * Compute Y correctly.
	 */
	if((op1==MOVEQ || (op1==MOVE && (i1->flags & LENL))) &&
	   sm1==IMM && dm1==REG && ISD(dr1) &&
	   OP_ARITHM(op2) && sm2==IMM && dm2==REG && dr2==dr1 && !i2->label)
		{long x;
		x = do_arithm(i1->src.disp,op2,i2->src.disp);
		if     (i2->flags & LENW) x &= 65535;
		else if(i2->flags & LENB) x &= 255;
		setupinst(i1,MOVE,LENL,IMM,x,REG,dr1);
		delinst(i2);
		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	move.l	#X,dx		=>	move.l	#Y,dx
	 *	swap	dx			<deleted>
	 *
	 * Compute Y correctly.
	 */
	if((op1==MOVEQ || (op1==MOVE && (i1->flags & LENL))) &&
	   sm1==IMM && dm1==REG && ISD(dr1) && op2==SWAP && sm2==REG &&
	   sr2==dr1 && !i2->label)
		{unsigned long x = i1->src.disp;
		setupinst(i1,MOVE,LENL,IMM,(((x>>16)&65535)|
					   ((x&65535)<<16)),REG,dr1);
		delinst(i2);
		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	move[q].l #X,dx 	=>	move.l	#Y,dx
	 *	not.x	  dx			<deleted>
	 *
	 * Compute Y correctly.
	 */
	if((op1==MOVEQ || (op1==MOVE && (i1->flags & LENL))) &&
	   sm1==IMM && dm1==REG && ISD(dr1) && op2==NOT && sm2==REG &&
	   sr2==dr1 && !i2->label)
		{long x = i1->src.disp;
		if     (i2->flags & LENB) x = ((-x)&255)|(x&~255);
		else if(i2->flags & LENW) x = ((-x)&65535)|(x&~65535);
		else x = -x;
		setupinst(i1,MOVE,LENL,IMM,x,REG,dr1);
		delinst(i2);
		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	bset.l	#b,dx		=>	or.l	#n,dx
	 *
	 * n = (1<<b)
	 */
	if(!size && op1==BSET && sm1==IMM && dm1==REG && ISD(dr1))
		{
		setupinst(i1,OR,LENL,IMM,(1<<i1->src.disp),REG,dr1);
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	pea	(Ax)            =>      move.l  Ax,-(sp)
	 */
	/* this is done in fixpea()
	if(op1==PEA && sm1==REGI)
		{
		setupinst(i1,MOVE,LENL,REG,sr1,REGI|DEC,SP);
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	move.l	#n,Dx		=>	<deleted>
	 *	INST.x	Dx,Rn			INST.x	#n,Rn
	 *
	 * Where Dx is dead.
	 * x = .w or .l. INST = cmp|and|or|sub|add|eor.
	 */
	if(OP_MOVE(op1) && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (OP_CMP(op2) || OP_ARITHM(op2)) && sm2==REG && sr2==dr1 &&
	   dm2==REG && (i2->flags & (LENL|LENW)) && !(i2->live & RM(dr1))
	   && !i2->label && !(i1->flags & (LENB|LENW)))
		{
		i2->src.amode = IMM;
		i2->src.disp  = i1->src.disp;
		delinst(i1);
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	sub.l	#n,Ax		=>	add.l	#-n,Ax
	 */
	if((op1==SUB || op1==SUBA) && sm1==IMM && dm1==REG && ISA(dr1) &&
	   (i1->flags&LENL))
		{
		i1->opcode   = op1==SUB?ADD:ADDA;
		i1->src.disp = -i1->src.disp;
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	INST1	<mem>		=>	INST1	<mem>
	 *	INST2	<mem>			INST3	<reg | imm>
	 *	INST3	<reg | imm>		INST2	<mem>
	 *
	 * Where INST1 and INST2 references memory and INST3 is only using
	 * regs or immediate data. Regs sets by INST3 must not be used by
	 * INST2. Regs sets by INST2 must not be used by INST3.
	 * UNSAFE: INST2 might set regs that are also set by INST3.
	 */
	if(is_mem_inst(i1) && is_mem_inst(i2) && is_no_mem_inst(i3) &&
	   !(i2->refs & i3->sets) && !(i3->refs& i2->sets) &&
	   !(i2->sets & i3->sets) &&
	   !breakflow(i3) && !breakflow(i2) && (!safe ||
	   (i3->next && uses_cc(i3->next->opcode))))
		{
		INST *i4 = i3->next;
		i1->next = i3;
		i3->next = i2;
		i2->next = i4;
		if(i4)
		i4->prev = i2;
		i2->prev = i3;
		i3->prev = i1;

		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	link	An,#0		=>	move.l	An,-(sp)
	 *					move.l	SP,An
	 */
	if(op1==LINK && dm1==IMM && !i1->dst.disp)
		{
		setupinst(i1,MOVE,LENL,REG,sr1,(REGI|DEC),SP);
		setupinst(insinst(i1),MOVE,LENL,REG,SP,REG,sr1);

		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	move	#n,Dy		=>	<deleted>
	 *	cmp.x	Dx,Dy			cmp.x	#n,Dx
	 *	b<cc>	lbl			b<cci>	lbl
	 *
	 * Where Dy is dead. b<cci> is the inverse branch.
	 */
	if(OP_MOVE(op1) && sm1==IMM && dm1==REG && ISD(dr1) &&
	   op2==CMP && sm2==REG && dm2==REG && dr2==dr1 &&
	   binv(op3) && !(i2->live & RM(dr1)))
		{
		setupinst(i2,CMPI,i1->flags,IMM,i1->src.disp,REG,sr2);
		i3->opcode = binv(op3);
		delinst(i1);

		++s_brev;
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	asl.x	#n,X		=>	lsl.x	#n,X
	 */
	if(op1==ASL)
		{
		i1->opcode=LSL;
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	INST (An)+,X            =>      INST (An),X
	 *
	 * Where An is dead.
	 */
	if (sm1==(REGI|INC) && !(i1->live&RM(sr1)))
		{
		i1->src.amode = REGI;
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	INST X,(An)+            =>      INST X,(An)
	 *
	 * Where An is dead.
	 */
	if (dm1==(REGI|INC) && !(i1->live&RM(dr1)))
		{
		i1->dst.amode = REGI;
		DBG(m4_line=__LINE__)
		goto true2;
		}
	/*
	 *	rts			=>	move.l	(sp)+,An
	 *					jmp	(An)
	 *
	 * Where An is dead.
	 */
	if(!size && op1==RTS && (i1->refs&ALLAREGS)!=ALLAREGS)
		{int An;
		for(An=A0;An<A7 && (i1->refs&RM(An));++An);
		setupinst(i1,MOVE,LENL,(REGI|INC),SP,REG,An);
		setupinst(insinst(i1),JMP,0,REGI,An,NONE);
		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	INST	..LBL(pc)..	=>	INST	..LBL..
	 */
	if(!size && (i1->src.amode==(PCD|SYMB)))
		{
		i1->src.amode=ABS|SYMB;
		DBG(m4_line=__LINE__)
		goto true;
		}
	if(!size && (i1->dst.amode==(PCD|SYMB)))
		{
		i1->dst.amode=ABS|SYMB;
		DBG(m4_line=__LINE__)
		goto true;
		}
	/*
	 *	pea	LBL		=>	move.l	#LBL,-(SP)
	 */
	if(op1==PEA && i1->src.amode==(ABS|SYMB))
		{
		i1->opcode    = MOVE;
		i1->flags     = LENL;
		i1->src.amode = IMM|SYMB;
		i1->dst.amode = REGI|DEC;
		i1->dst.areg  = SP;
		DBG(m4_line=__LINE__)
		goto true;
		}
	 	
	return NULL;
true:	return i1->next;
true2:	return i2;
	}

/*.
 * mc40opt() - process 68040 optimisation of source
 */
bool mc40opt()
	{
	register INST	*ip,*ni;
	register bool	changed = FALSE;

	DBG(printf("mc40opt  : "))
	for(ip=head; ip ; )
		{int line=ip->lineno;
		if(ni = ipeep(ip))
			{
			changed = TRUE;
			++s_mc40;
			ip = ni;
			DBG(printf("%d(%d) ",m4_line,line);fflush(stdout))
			}
		else	ip=ip->next;
		}
	DBG(printf("\n"); fflush(stdout))
	return changed;
	}
