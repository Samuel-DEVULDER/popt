/*
 *	mc20opt.c - some optimisations for 68030/020.
 *
 *	Suggestions by Pascal Lauly (lauly@cnam.fr) and Loic Marechal
 *	(marechal@cnam.fr). Thanks Pascal & Loic.
 */
/*.	(c) by Samuel DEVULDER
 */

#include "popt.h"

static int m2_line;

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
 *	is_mem_write_inst() - is instruction referencing memory for writing ?
 */
static bool is_mem_write_inst(ip)
	INST *ip;
	{
	if(!ip || !is_real_inst(ip->opcode) || ip->opcode==LEA)
		return FALSE;
	return is_mem_opnd(&ip->dst);
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
 *	ipeep(i1) - performe optimizations for 68020 for instructions
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
	 *	OPTIMIZATIONS FOR 68020:
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
		DBG(m2_line=__LINE__);
		goto true;
		}
	/*
	 *	add.w	#n,Ax		=>	add.l	#n,Ax
	 *
	 * Add or sub.
	 */
	if((OP_ADD(op1)||OP_SUB(op1)) && (i1->flags==LENW) && M(sm1)==IMM &&
	   dm1==REG && ISA(dr1))
		{
		i1->flags = LENL;
		DBG(m2_line=__LINE__);
		goto true2;
		}
	/*
	 *	link	An,#0		=>	move.l	An,-(sp)
	 *					move.l	SP,An
	 */
	if(op1==LINK && dm1==IMM && !i1->dst.disp)
		{
		setupinst(i1,MOVE,LENL,REG,sr1,(REGI|DEC),SP);
		setupinst(insinst(i1),MOVE,LENL,REG,SP,REG,sr1);

		DBG(m2_line=__LINE__)
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
		DBG(m2_line=__LINE__)
		goto true;
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
			DBG(m2_line=__LINE__)
			goto true2;
			}
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
	   (op1==ASL || op1==LSL) && sm1==IMM && dm1==REG && ISD(dr1)
	   && i1->src.disp>=1 && i1->src.disp<=3 && i1->flags==LENL)
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
			DBG(m2_line=__LINE__)
			goto true2;
			}
	   	}
end1:	/*.
	 *	lea	n(Ax),Ax        =>      adda.l  #n,Ax
	 *	4/0/4				4/0/6
	 * Not for size optimisation.
	 * /
	if(!size && op1==LEA && sm1==REGID && sr1==dr1)
		{
		setupinst(i1,ADDA,LENL,IMM,i1->src.disp,REG,sr1);
		DBG(m2_line=__LINE__)
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
		DBG(m2_line=__LINE__)
		goto true2;
		}
	/*
	 *	INST	0(Am,Rn.l),X	=>	add.l   Rn,Am
	 *					INST	(Am),X
	 *
	 * Where Am is dead. X must not refer Am.
	 */
	if (last_pass && sm1==(REGIDX|XLONG) && i1->src.disp==0 &&
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
			DBG(m2_line=__LINE__)
			goto true2;
			}
		}
	/*
	 *	INST	X,0(Am,Rn.l)	=>	add.l   Rn,Am
	 *					INST	X,(Am)
	 *
	 * Where Am is dead. X must not refer Am.
	 */
	if (last_pass && dm1==(REGIDX|XLONG) && i1->dst.disp==0 &&
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
			DBG(m2_line=__LINE__)
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
		DBG(m2_line=__LINE__)
		goto true2;
		}
	/*
	 *	bclr.l	#b,dx		=>	and.l	#n,dx
	 *
	 * n = ~(1<<b) (not for size optimisations).
	 */
	if(!size && op1==BCLR && sm1==IMM && dm1==REG && ISD(dr1))
		{
		setupinst(i1,AND,LENL,IMM,~(1<<i1->src.disp),REG,dr1);
		DBG(m2_line=__LINE__)
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
		DBG(m2_line=__LINE__)
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
		DBG(m2_line=__LINE__)
		goto true;
		}
	/*
	 *	move.l	#X,dx		=>	move.l	#Y,dx
	 *	not.x	dx			<deleted>
	 *
	 * Compute Y correctly.
	 */
	if(op1==MOVE && (i1->flags & LENL) &&
	   sm1==IMM && dm1==REG && ISD(dr1) && op2==NOT && sm2==REG &&
	   sr2==dr1 && !i2->label)
		{long x = i1->src.disp;
		if     (i2->flags & LENB) x = ((-x)&255)|(x&~255);
		else if(i2->flags & LENW) x = ((-x)&65535)|(x&~65535);
		else x = -x;
		setupinst(i1,MOVE,LENL,IMM,x,REG,dr1);
		delinst(i2);
		DBG(m2_line=__LINE__)
		goto true;
		}
	/*
	 *	bset.l	#b,dx		=>	or.l	#n,dx
	 *
	 * n = (1<<b) (not for size opt.)
	 */
	if(!size && op1==BSET && sm1==IMM && dm1==REG && ISD(dr1))
		{
		setupinst(i1,OR,LENL,IMM,(1<<i1->src.disp),REG,dr1);
		DBG(m2_line=__LINE__)
		goto true2;
		}
	/*
	 *	move.l	Ax,Dy		=>	add.l	Ax,Ax
	 *	lsl.l	#3,Dy			add.l	Ax,Ax
	 *	move.l	Dy,Ax			add.l	Ax,Ax
	 *
	 * Where Dy is dead. UNSAFE: lsl sets the flags. add does'nt.
	 */
	if((!safe || cc_modified(i3->next)) &&
	   op1==ADD && (op2==LSL || op2==ASL) && op3==MOVE &&
	   i1->flags==i2->flags && i2->flags==i3->flags && i1->flags==LENL &&
	   sm1==REG && ISA(sr1) && dm1==REG && ISD(dr1) &&
	   sm2==IMM && i2->src.disp==2 && dm2==REG && dr2==dr1 &&
	   sm3==REG && sr3==dr1 && dm3==REG && dr3==sr1 &&
	   !(i3->live & RM(dr1)))
		{
		setupinst(i1,ADD,LENL,REG,sr1,REG,dr1);
		setupinst(i2,ADD,LENL,REG,sr1,REG,dr1);
		setupinst(i3,ADD,LENL,REG,sr1,REG,dr1);

		DBG(m2_line=__LINE__)
		goto true2;
		}
	/*
	 *	INST1	<mem write>	=>	INST1	<mem write>
	 *	INST2	<mem>			INST3	<reg | imm>
	 *	INST3	<reg | imm>		INST2	<mem>
	 *
	 * Where INST1 write to memory, INST2 references memory for
	 * reading or writing and INST3 is only using regs or immediate
	 * data. Regs sets by INST3 must not be used by INST2. Regs sets
	 * by INST2 must not be used by INST3.
	 * UNSAFE: INST2 might set regs that are also set by INST3.
	 */
	if(is_mem_write_inst(i1) && is_mem_inst(i2) && is_no_mem_inst(i3) &&
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

		DBG(m2_line=__LINE__)
		goto true;
		}
	return NULL;
true:	return i1->next;
true2:	return i2;
	}

/*.
 * mc20opt() - process 68020 optimisation of source
 */
bool mc20opt()
	{
	register INST	*ip,*ni;
	register bool	changed = FALSE;

	DBG(printf("mc20opt  : "))
	for(ip=head; ip ; )
		{int line=ip->lineno;
		if(ni = ipeep(ip))
			{
			changed = TRUE;
			++s_mc20;
			ip = ni;
			DBG(printf("%d(%d) ",m2_line,line);fflush(stdout))
			}
		else	ip=ip->next;
		}
	DBG(printf("\n"); fflush(stdout))
	return changed;
	}
