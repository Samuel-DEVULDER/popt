/*.
 *	peep1.c - Single-instruction peephole optimizations and the overall
 *		  driver routine.
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

static bool peep1();

void peep()
	{
	INST	*ip;
	register bool	changed;

	/*.
	 * Loop until no more changes are made. After each change, do
	 * live/dead analysis or the data gets old. In each loop, make
	 * at most one change.
	 */
/*	putsource("*"); /**/
	do	{
		changed  = peep4();
		changed |= peep3();
		changed |= peep2();
/*		putsource("*");	/**/
		changed |= stackopt();
		changed |= peep1();
		changed |= asp68k();
		if(mc60) changed |= mc60opt();
		if(mc40) changed |= mc40opt();
		if(mc20) changed |= mc20opt();
		if(changed) {branchopt();do_health();}
/*		putsource("*"); /**/
		}
	while(changed);
	}

/*.
 *	fix_pea() -    pea     (An)    =>      move.l  An,-(sp)
 */
void fix_pea()
	{
	INST *ip;
	for(ip=head;ip;ip=ip->next) 
		{
		if(ip->opcode==PEA && ip->src.amode==REGI)
			{
			ip->opcode    = MOVE;
			ip->flags     = LENL;
			ip->src.amode = REG;
			ip->dst.amode = REGI|DEC;
			ip->dst.areg  = SP;
			}
		}
	}

/*.
 *	nb_bits(n) - returns number of bit set in n
 */
int nb_bits(n)
	unsigned int n;
	{
	int j=0;
	n&=65535;
	while(n) {if(n&1) ++j;n>>=1;}
	return j;
	}

/*.
 *	first_nb(n) - returns the index of the first bit set in n
 */
int first_nb(n)
	unsigned int n;
	{
	int j = 0;
	while(!(n & 1)) {n >>= 1;++j;}
	return j;
	}


/*.
 *	OP_ARITHM_UN() - is op a unary arithmetic operation
 */
bool OP_ARITHM_UN(op)
	int op;
	{
	switch(op)
		{
		case NEGX: case SCC: case SCS: case SEQ: 
		case SGE:  case SGT: case SHI: case SLE:
		case SLS:  case SLT: case SMI: case SNE:
		case SPL:  case SVC: case SVS: case SWAP:
		case TST:  case NEG: case NOT: case SF: 
		case ST:   case EXT: case EXTB: case CLR:
		return TRUE;

		default: 
		return FALSE;
		}
	}

static int p1_line;

/*.
 * ipeep1(ip) - check for changes to the instruction 'ip' return next inst
 */
static INST *ipeep1(ip)
	register INST	*ip;
	{
	int op,sm,dm,sr,dr,fl;
	INST *ti;

	op = ip->opcode;
	sm = ip->src.amode;
	sr = ip->src.areg;
	dm = ip->dst.amode;
	dr = ip->dst.areg;

	/*
	 *	clr.l  Dn		=>	moveq  #0,Dn
	 *
	 * For 68020/30 this does not improve speed but may help for
	 * further optimisations.
	 */
	if (op == CLR && sm == REG && ISD(sr) && (ip->flags==LENL))
		{
		setupinst(ip,MOVEQ,0,IMM,0,REG,sr);
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*.
	 *	clr.x  Ay		=>	sub.x  Ay,Ay
	 *	 ^
	 *	produced by the optimizer.
	 */
	if (op == CLR && sm == REG && ISA(sr))
		{
		ip->opcode = SUB;
		ip->dst    = ip->src;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	move.x	#n,Dn		=>	moveq  #n,Dn
	 *
	 * Moveq is always a long operation, but as long as the
	 * immediate value is appropriate, we don't care what the
	 * original length was. Clearing upper bytes won't matter.
	 * This might cause a bug !
	 * Not for 060.
	 */
	if (OP_MOVE(op) && (op!=MOVEQ) && sm == IMM && ISD(dr) &&
	    D8OK(ip->src.disp) && !mc60)
		{
		ip->opcode = MOVEQ;
		ip->flags  = 0;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	move.x	#0,X		=>	clr.x	X
	 *
	 * X!=Rn. Not for 68000 (unsafe and no speed gain) except
	 * when size opt is required.
	 */
	if ((mc20 || mc40 || mc60 || (safe && size)) && OP_MOVE(op) && 
	    (op!=MOVEQ) && sm == IMM && 
	    !ip->src.disp && ip->dst.amode!=REG)
		{
		ip->opcode    = CLR;
		ip->src       = ip->dst;
		ip->dst.amode = NONE;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	and.l #$ffffxxxx,Dx	=>	and.w #xxxx,Dx
	 */
	if(OP_AND(op) && sm==IMM && dm==REG && ISD(dr) && ip->flags==LENL &&
	   ((ip->src.disp&0xffff0000)==0xffff0000) )
		{
		ip->src.disp &= 65535;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	or.l #$0000xxxx,Dx	=>	or.w #xxxx,Dx
	 *
	 * OR or EOR.
	 */
	if((OP_OR(op) || OP_EOR(op)) && sm==IMM && dm==REG && ISD(dr) &&
	   ip->flags==LENL && !(ip->src.disp&0xffff0000))
		{
		ip->src.disp &= 65535;
		ip->flags=LENW;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	and.l #65535,Dx 	=>	swap  dx
	 *					clr.w dx
	 *					swap  dx
	 * Not for 68020+.
	 */
	if(!mc60 && !mc40 && !mc20 && OP_AND(op) && sm==IMM &&
	   ip->src.disp==65535 && dm==REG && ISD(dr) &&
	   ip->flags==LENL)
		{INST *ni;
		setupinst(ip,                   SWAP,0,REG,dr,NONE);
		ni=setupinst(insinst(ip),       CLR,LENW,REG,dr,NONE);
		ni=setupinst(insinst(ni),       SWAP,0,REG,dr,NONE);
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	add.l #n,An		=>	add.w #n,An
	 *
	 * Where -32768<=n<=32767 and add or sub. Not for 68020+.
	 */
	if(!mc60 && !mc40 && !mc20 &&
	   (OP_ADD(op) || OP_SUB(op)) && sm==IMM && dm==REG && ISA(dr) &&
	   (ip->flags & LENL) && DOK(ip->src.disp))
		{
		ip->flags = LENW;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	add.x  #n, X		=>	addq.x	#n, X
	 *
	 * Where 1 <= n <= 8.
	 */
	if (OP_ADD(op) && (op!=ADDQ) && sm == IMM &&
	    ip->src.disp >= 1 && ip->src.disp <= 8)
		{
		ip->opcode = ADDQ;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	add.x  #n, X		=>	subq.x	#-n, X
	 *
	 * Where -8 <= n <= -1. Not for 040+
	 */
	if (OP_ADD(op) && sm == IMM && !mc60 && !mc40 &&
	    ip->src.disp <= -1 && ip->src.disp >= -8)
		{
		ip->opcode   = SUBQ;
		ip->src.disp = -ip->src.disp;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	sub.x  #n, X		=>	subq.x	#n, X
	 *
	 * Where 1 <= n <= 8.
	 */
	if (OP_SUB(op) && (op!=SUBQ) && sm == IMM &&
	    ip->src.disp >= 1 && ip->src.disp <= 8)
		{
		ip->opcode = SUBQ;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	sub.x  #n, X		=>	addq.x	#-n, X
	 *
	 * Where -8 <= n <= -1. Not for 060.
	 */
	if (OP_SUB(op) && sm == IMM && !mc60 &&
	    ip->src.disp >= -8 && ip->src.disp <= -1)
		{
		ip->opcode   = ADDQ;
		ip->src.disp = -ip->src.disp;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	movem.y	X,Y		=>	<deleted>
	 *
	 * delete if mask is empty
	 */
	if(op==MOVEM && ((sm==MSK && !ip->src.disp)||
			 (dm==MSK && !ip->dst.disp)))
		{ti=ip->next;
		delinst(ip);
		DBG(p1_line=__LINE__)
		goto true2;
		}

	/*
	 * Delete instruction that sets CC when CC is dead:
	 */

	/*
	 *	CMP	X,Y		=>	<deleted>
	 *
	 * Delete if CC is dead and X or Y must not be INC or DEC. UNSAFE.
	 */
	if (OP_CMP(op) && !(sm & (DEC|INC)) && !(dm & (DEC|INC)) && 
	    is_cc_dead(ip))
		{
		ti = ip->next;
		delinst(ip);
		DBG(p1_line=__LINE__)
		goto true2;
		}
	/*
	 *	TST	X		=>	<deleted>
	 *
	 * Delete if CC is dead and X must not be INC or DEC. UNSAFE.
	 */
	if (op==TST && !(sm & (DEC|INC)) && is_cc_dead(ip))
		{
		ti = ip->next;
		delinst(ip);
		DBG(p1_line=__LINE__)
		goto true2;
		}
	/*
	 *	ARITHM	X,Rn		=>	<deleted>
	 *
	 * Remove instruction if Rn and CC are dead. This is most often
	 * used to eliminate the fixup of SP following a function call 
	 * when we're just about to return, since the "unlk" clobbers SP
	 * anyway. X must not be INC or DEC. ARITHM can also be a MOVE.
	 */
	if ((OP_ARITHM(op) || OP_MOVE(op)) && !(sm & (DEC|INC)) && 
	    dm == REG && (ISA(dr) || is_cc_dead(ip)) && !(ip->live & RM(dr)))
		{
		ti = ip->next;
		delinst(ip);
		DBG(p1_line=__LINE__)
		goto true2;
		}
	/*
	 *	ARITHM	Rn		=>	<deleted>
	 *
	 * Remove instruction if Rn and CC are dead.
	 */
	if (OP_ARITHM_UN(op) && sm == REG && is_cc_dead(ip) &&
	    !(ip->live & RM(sr)))
		{
		ti = ip->next;
		delinst(ip);
		DBG(p1_line=__LINE__)
		goto true2;
		}
	/*
	 *	move.x	X,X		=>	tst.x	X
	 *
	 * If X isn't INC or DEC. (delete if X=An).
	 */
	if((op == MOVE) && opeq(&ip->src,&ip->dst) && !(sm & (INC|DEC)))
		{
		ti = ip->next;
		ip->opcode = TST;
		if(sm==REG && ISA(dr)) delinst(ip);
		else	{freeop(&ip->dst);ip->dst.amode=NONE;}
		DBG(p1_line=__LINE__)
		goto true2;
		}
	/*
	 *	add[ai].x  #n, Am	=>	lea  n(Am), Am
	 *
	 * Where 'n' is a valid displacement. UNSAFE since lea doesnt
	 * set the flags. Not for 68040.
	 */
	if((!safe || cc_modified(ip->next)) && !mc40 && OP_ADD(op) &&
	   op!=ADDQ && sm == IMM && dm == REG && ISA(dr) &&
	   DOK(ip->src.disp))
		{
		setupinst(ip,   LEA,0,REGID,ip->src.disp,dr,REG,dr);
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	lea  n(Am),Am           =>      addq.w #n,Am    if n>0
	 *					subq.w #-n	if n<0
	 *
	 * Where -8<=n<=8. UNSAFE since addq sets flags.
	 */
	if((!safe || cc_modified(ip->next)) && op==LEA && sm == REGID &&
	   sr==dr && dm == REG &&
	   ISA(dr) && (-8<=ip->src.disp) && (ip->src.disp<=8))
		{
		setupinst(ip,(ip->src.disp>=0?ADDQ:SUBQ),LENW,IMM,
			(ip->src.disp>=0?ip->src.disp:-ip->src.disp),REG,dr);
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	cmp.x	#0, X		=>	tst.x	X
	 *
	 * Where X is not An.
	 */
	if(OP_CMP(op) && ((dm!=REG) || !ISA(dr)) && sm==IMM && !ip->src.disp)
		{
		ip->opcode    = TST;
		ip->src       = ip->dst;
		ip->dst.amode = NONE;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	cmp.l	#0,An		=>	move.l	An,Ds
	 *
	 * Where Ds is dead.
	 */
	if(OP_CMP(op) && dm==REG && ISA(dr) && sm==IMM && !ip->src.disp &&
	   ip->flags==LENL && ((ip->live&ALLDREGS)!=ALLDREGS))
		{int Ds;
		for(Ds=D0;Ds<=D7 && (ip->live&RM(Ds));++Ds);
		setupinst(ip,MOVE,LENL,REG,dr,REG,Ds);
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	cmp.l	#0,An		=>	cmp.w	#0,An
	 */
	if(OP_CMP(op) && dm==REG && ISA(dr) && sm==IMM && !ip->src.disp &&
	   ip->flags==LENL)
		{
		ip->flags = LENW;
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	cmp.x	#y, Rm		=>	subq.x	#abs(y),Rm   (y>0)
	 *					addq		     (y<0)
	 *
	 * Where Rm is dead and (1 <= abs(y) <= 8).
	 */
	if(OP_CMP(op) && (sm == IMM) && (dm == REG) && !(ip->live & RM(dr))
	   && (ip->src.disp >= -8) && (ip->src.disp <= 8) && ip->src.disp)
		{
		if(ip->src.disp>0)
			ip->opcode   = SUBQ;
		else	{
			ip->opcode   = ADDQ;
			ip->src.disp = -ip->src.disp;
			}
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	lsl.x	#1,Dx		=>	add.x	Dx,Dx
	 *	asl.x
	 * Not for 060.
	 */
	if((op==LSL || op==ASL) && (sm==IMM) && (ip->src.disp==1) &&
	   (dm==REG) && ISD(dr) && !mc60)
		{
		setupinst(ip,ADD,ip->flags,dm,dr,dm,dr);
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	move.l	An,-(SP)        =>      pea     (An)
	 *
	 * Will help further optimizations with INDEX mode. Will be set back
	 * to original code if useless (ie. 68020+).
	 */
	if(op==MOVE && sm==REG && ISA(ip->src.areg) && dm==(REGI|DEC) &&
	   dr==SP && ip->flags==LENL)
		{
		setupinst(ip,PEA,0,REGI,ip->src.areg,NONE);
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*
	 *	lea	(An),Am		=>	move.l	An,Am
	 */
	if(op==LEA && sm==REGI)
		{
		setupinst(ip,MOVE,LENL,REG,ip->src.areg,REG,ip->dst.areg);
		DBG(p1_line=__LINE__)
		goto true;
		}
	/*.
	 * The following optim. must be put after add #n,An => lea n(An),An
	 * for 68000.
	 */
	/*
	 *	INST.x	#n,X		=>	moveq	#n,Ds
	 *					INST.x	Ds,X
	 *
	 * Where Ds is a scratch reg, INST=ADD | ADDA | ADDI | AND | ANDI |
	 * OR | ORI | EOR | EORI | CMP | CMPA | SUB | SUBI | SUBA | MOVE |
	 * MOVEA. Not for 68040 or 060.
	 */
	if(!mc60 && !mc40 && sm==IMM && D8OK(ip->src.disp) &&
	   ((ip->live|ip->refs)&ALLDREGS)!=ALLDREGS &&
	   (op==ADD  || op==ADDA || op==ADDI || op==AND  || op==ANDI ||
	    op==OR   || op==ORI  || op==EOR  || op==EORI ||
	    ((op==CMP|| op==CMPI) && ip->dst.amode==REG) ||
	    op==CMPA || op==SUB  || op==SUBI || op==SUBA || op==MOVE ||
	    op==MOVEA))
		{int Ds;
		for(Ds=D0;Ds<=D7 && ((ip->live|ip->refs)&RM(Ds));++Ds);
		setupinst(inspreinst(ip),MOVEQ,0,IMM,ip->src.disp,REG,Ds)->live|=RM(Ds);
		ip->src.amode=REG;ip->src.areg=Ds;
		if(op==ADDI || op==ADDQ) ip->opcode=ADD;
		if(op==SUBI || op==SUBQ) ip->opcode=SUB;
		if(op==ORI)  ip->opcode=OR;
		if(op==EORI) ip->opcode=EOR;
		if(op==ANDI) ip->opcode=AND;
		if(op==CMPI) ip->opcode=CMP;
		DBG(p1_line=__LINE__)
		goto true;
		}

	return NULL;

true:	return ip->next;	/* std return */
true2:	return ti;		/* return when deletion */
	}

/*.
 * peep1() - peephole optimizations with a window size of 1
 */
static bool peep1()
	{
	register INST	*ip,*ni;
	register bool	changed = FALSE;

	DBG(printf("peep (1) : "))
	for(ip=head; ip ; )
		{int line=ip->lineno;
		if(ni = ipeep1(ip))
			{
			++s_peep1;
			DBG(printf("%d(%d) ",p1_line,line);fflush(stdout))
			changed = TRUE;
			ip = ni;
			}
		else	ip=ip->next;
		}
	DBG(printf("\n"); fflush(stdout))
	return changed;
	}
