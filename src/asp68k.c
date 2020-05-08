/*.
 *	asp68k.c - some optimisations from asp68k project..
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

int a_line;

/*.
 * ipeep(ip) - check for changes to the instruction 'ip' return next inst
 */
static INST *ipeep(i1)
	register INST	*i1;
	{
	int op1,sm1,dm1,sr1,dr1,fl;
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
	else
		{
		op2 = UNKWN;
		sm2 = NONE;
		dm2 = NONE;
		}

	/*
	 *	INST 0(An),X            =>      INST (An),X
	 */
	if (sm1==REGID && i1->src.disp==0)
		{
		i1->src.amode = REGI;
		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	INST X,0(An)            =>      INST X,(An)
	 */
	if (dm1==REGID && i1->dst.disp==0)
		{
		i1->dst.amode = REGI;
		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	add*.x #0,Dx		=>	tst.x Dx
	 * Not for 060.
	 */
	if (OP_ADD(op1) && sm1==IMM && dm1==REG && i1->src.disp==0 &&
	    ISD(dr1) && !mc60)
		{
		i1->opcode    = TST;
		i1->src.amode = REG;
		i1->src.areg  = dr1;
		i1->dst.amode = NONE;
		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	add.x #I1,Rm		=>	add.x #I1+I2,Rm
	 *	...				...
	 *	<stuff that does not use Rm>	<stuff>
	 *	...				...
	 *	add.x #I2,Rm			<deleted>
	 *
	 * Add or sub.. UNSAFE since the 2nd add set the flags so test
	 * if inst after last add set flags.
	 */
	if ((OP_ADD(op1)||OP_SUB(op1)) && sm1==IMM && dm1==REG)
		{ INST *ni = i1->next;
		while(ni && !breakflow(ni) && !uses(ni,dr1)) ni = ni->next;
		if(ni && uses(ni,dr1) && !ni->label &&
		   (OP_ADD(ni->opcode)||OP_SUB(ni->opcode)) &&
		   ni->src.amode == IMM && ni->dst.amode == REG &&
		   ni->dst.areg == dr1 && (!safe || cc_modified(ni->next)))
			{
			if(OP_ADD(op1))
				i1->src.disp += OP_ADD(ni->opcode)?
						ni->src.disp:-ni->src.disp;
			else	i1->src.disp -= OP_ADD(ni->opcode)?
						ni->src.disp:-ni->src.disp;
			if(i1->src.disp>8 || i1->src.disp<-8)
				{
				if(op1==ADDQ)           i1->opcode=ADDI;
				else if(op1==SUBQ)      i1->opcode=SUBI;
				}
			delinst(ni);
			DBG(a_line=__LINE__)
			goto true;
			}
		}
	/*
	 *	addq.x #4,sp		=>	move.l ax,(sp)
	 *	pea (ax)                        <deleted>
	 *
	 * Where x = w or l
	 */
	if (OP_ADD(op1) && sm1==IMM && dm1==REG && (i1->flags & (LENL|LENW)) &&
	    i1->src.disp==4 && dr1==SP && op2==PEA && sm2==REGI && sr2!=A7 &&
	    !i2->label)
		{
		setupinst(i1,MOVE,LENL,REG,sr2,REGI,SP);
		delinst(i2);
		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	and.l #n,dx		=>	bclr.l #b,dx
	 *
	 * Where not(n) = 2^b (only 1 bit off). Not for 68020+.
	 */
	if(!mc60 && !mc40 && !mc20 && op1==AND && (i1->flags&LENL) &&
	   sm1==IMM && dm1==REG &&
	   ISD(dr1) && nb_bits(~i1->src.disp)==1)
		{
		i1->opcode   = BCLR;
		i1->src.disp = first_nb(~i1->src.disp);
		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	asl.b #n,dx		=>	clr.b dx
	 *
	 * For asr or lsl or lsr also. Where n>=8. UNSAFE: status flags are
	 * wrong so test if next instruction sets them. Not for 060.
	 */
	if((!safe || cc_modified(i2)) && !mc60 &&
	   (op1==ASL || op1==ASR || op1==LSL || op1==LSR) && sm1==IMM &&
	   dm1==REG && ISD(dr1) &&
	   (i1->src.disp >= 8) && (i1->flags & LENW))
		{
		setupinst(i1,CLR,LENB,REG,dr1,NONE);

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	asl.l #16,dx		=>	swap  dx
	 *					clr.w dx
	 *
	 * For lsl also. UNSAFE: status flags are wrong so test if next
	 * instruction sets them. Not for 68020+.
	 */
	if((!safe || cc_modified(i2)) && !mc20 && !mc40 && !mc60 &&
	   (op1==ASL || op1==LSL) && sm1==IMM && dm1==REG && ISD(dr1) &&
	   i1->src.disp == 16 && (i1->flags & LENL))
		{
		setupinst(i1,SWAP,0,REG,dr1,NONE);
		setupinst(insinst(i1),CLR,LENW,REG,dr1,NONE);

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	asr.l #16,dx		=>	clr.w dx
	 *					swap  dx
	 *
	 * For lsr also. UNSAFE: status flags are wrong so test if next
	 * instruction sets them. Not for 68020+.
	 */
	if((!safe || cc_modified(i2)) && !mc20 && !mc40 && !mc60 &&
	   (op1==ASR || op1==LSR) && sm1==IMM && dm1==REG && ISD(dr1) &&
	   i1->src.disp == 16 && (i1->flags & LENL))
		{
		setupinst(i1,SWAP,0,REG,dr1,NONE);
		setupinst(inspreinst(i1),CLR,LENW,REG,dr1,NONE);

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	asl.l #n,dx		=>	asl.w #(n-16),dx
	 *					swap dx
	 *					clr.w dx
	 *
	 * Where asl or lsl and 16<n<32. UNSAFE: status flags are wrong
	 * so test if next instruction sets them. Not for 68020+.
	 */
	if(!mc60 && !mc40 && !mc20 && !size && (!safe || cc_modified(i2)) &&
	   (op1==ASL || op1==LSL) && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->src.disp > 16) && (i1->src.disp<32) && (i1->flags & LENL))
		{ INST *ni;
		i1->flags     = LENW;
		i1->src.disp -= 16;

		ni = setupinst(insinst(i1),SWAP,0,REG,dr1,NONE);
		ni = setupinst(insinst(ni),CLR,LENW,REG,dr1,NONE);

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	asl.l #n,dx		=>	moveq #0,dx
	 *
	 * Where asl or lsl or asr or lsr and 32<=n. UNSAFE: status flags
	 * are wrong so test if next instruction sets them. Not for 060.
	 */
	if((!safe || cc_modified(i2)) && !mc60 &&
	   (op1==ASL || op1==ASR || op1==LSL || op1==LSR) && sm1==IMM &&
	   dm1==REG && ISD(dr1)&&(i1->src.disp >= 32) && (i1->flags & LENL))
		{
		i1->opcode   = MOVEQ;
		i1->src.disp = 0;

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	asl.w #n,dx		=>	clr.w dx
	 *
	 * Where asl or lsl or asr or lsr and 16<=n. UNSAFE: status flags
	 * are wrong so test if next instruction sets them. Not for 060.
	 */
	if((!safe || cc_modified(i2)) && !mc60 &&
	   (op1==ASL || op1==ASR || op1==LSL || op1==LSR) && sm1==IMM &&
	   dm1==REG && ISD(dr1) &&
	   (i1->src.disp >= 16) && (i1->flags & LENW))
		{
		setupinst(i1,CLR,LENW,REG,dr1,NONE);

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	asr.l #n,dx		=>	swap  dx
	 *					asr.w #(n-16),dx
	 *					clr.w dx
	 *
	 * Where asl or lsl and 16<n<32. UNSAFE: status flags are wrong
	 * so test if next instruction sets them. Not for 68020+.
	 */
	if(!mc60 && !mc40 && !mc20 && !size && (!safe || cc_modified(i2)) &&
	   (op1==ASR || op1==LSR) && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->src.disp > 16) && (i1->src.disp<32)  &&
	   (i1->flags & LENL))
		{ INST *ni;
		i1->flags     = LENW;
		i1->src.disp -= 16;

		ni = setupinst(inspreinst(i1),SWAP,0,REG,dr1,NONE);
		ni = setupinst(insinst(i1),CLR,LENW,REG,dr1,NONE);

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	bclr.l #n,dx		=>	and.w #m,dx
	 *
	 * Where 0 <= n <= 15, m = 65535-(2^n). UNSAFE: status flags
	 * are wrong so test if next instruction sets them. Not for 060.
	 */
	if((!safe || cc_modified(i2)) && !mc60 &&
	   op1==BCLR && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->flags & LENL) && (0<=i1->src.disp) && (i1->src.disp<=15))
		{
		i1->opcode   = AND;
		i1->flags    = LENW;
		i1->src.disp = 65535^(1<<i1->src.disp);

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	bset.l #n,dx		=>	or.w #m,dx
	 *
	 * Where 0 <= n <= 15, m = (2^n). UNSAFE: status flags are wrong
	 * so test if next instruction sets them. Not for 060.
	 */
	if((!safe || cc_modified(i2)) && !mc60 &&
	   op1==BCLR && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->flags & LENL) && (0<=i1->src.disp) && (i1->src.disp<=15))
		{
		i1->opcode   = OR;
		i1->flags    = LENW;
		i1->src.disp = 1<<i1->src.disp;

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	btst.l	  #7,dx 	=>	tst.b	  dx
	 *	beq | bne ??			bpl | bmi ??
	 *
	 * UNSAFE: btst just sets Z while tst sets N also. So test if
	 * next instruction sets the flags.
	 */
	if(op1==BTST && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->flags & LENL) && i1->src.disp == 7 &&
	   (op2==BEQ || op2==BNE) && (!safe || cc_modified(i2->next)))
		{
		setupinst(i1,TST,LENB,REG,dr1,NONE);

		i2->opcode    = op2==BEQ?BPL:BMI;

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	btst.l	  #15,dx	=>	tst.w	  dx
	 *	beq | bne ??			bpl | bmi ??
	 *
	 * UNSAFE: btst just sets Z while tst sets N also. So test if
	 * next instruction sets the flags. Not for 060.
	 */
	if(op1==BTST && sm1==IMM && dm1==REG && ISD(dr1) && !mc60 &&
	   (i1->flags & LENL) && i1->src.disp == 15 &&
	   (op2==BEQ || op2==BNE) && (!safe || cc_modified(i2->next)))
		{
		setupinst(i1,TST,LENW,REG,dr1,NONE);

		i2->opcode    = op2==BEQ?BPL:BMI;

		DBG(a_line=__LINE__)
		goto true2;
		}
	/*
	 *	btst.l	  #31,dx	=>	tst.l	  dx
	 *	beq | bne ??			bpl | bmi ??
	 *
	 * UNSAFE: btst just sets Z while tst sets N also. So test if
	 * next instruction sets the flags. Not for 060.
	 */
	if(op1==BTST && sm1==IMM && dm1==REG && ISD(dr1) && !mc60 &&
	   (i1->flags & LENL) && i1->src.disp == 31 &&
	   (op2==BEQ || op2==BNE) && (!safe || cc_modified(i2->next)))
		{
		setupinst(i1,TST,LENL,REG,dr1,NONE);

		i2->opcode    = op2==BEQ?BPL:BMI;

		DBG(a_line=__LINE__)
		goto true2;
		}

	/*
	 *	No div optimisation because of the remainder..
	 */
	return peepabis(i1);

true:	return i1->next;			/* return when deletion */
true2:	return i2;				/* std return */
	}

/*.
 * asp68k() - process asp68k optimisation of source
 */
bool asp68k()
	{
	register INST	*ip,*ni;
	register bool	changed = FALSE;

	DBG(printf("asp68k   : "))
	for(ip=head; ip ; )
		{int line=ip->lineno;
		if(ni = ipeep(ip))
			{
			DBG(printf("%d(%d) ",a_line,line);fflush(stdout))
			changed = TRUE;
			++s_asp68k;
			ip = ni;
			}
		else	ip=ip->next;
		}
	DBG(printf("\n"); fflush(stdout))
	return changed;
	}
