/*.
 *	peep2_5.c - 2-instruction peephole optimizations (following)
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

extern int p2_line;

/*.
 * ipeep2_5(i1) - look for 2-instruction optimizations at the given 
 *		  inst. Part V.
 */
INST *ipeep2_5(i1)
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
	 *	move.x	X,Dx		=>	moveq	#0,Dx
	 *	and.l	#65535,Dx		move.y	X,Dx
	 *
	 * Where X is REG or IMM. If x=.l then y=.w else y=x.
	 */
	if(OP_MOVE(op1) && (sm1==IMM||sm1==REG) && dm1==REG &&
	   OP_AND(op2) && i2->src.amode==IMM && i2->dst.amode==REG && 
	   i2->src.areg==dr1 && i2->src.disp==65535 && i2->flags==LENL)
		{
		setupinst(inspreinst(i1),MOVEQ,0,IMM,0,REG,dr1);
		if(i1->flags==LENL) i1->flags=LENW;
		delinst(i2);
		
		DBG(p2_line=__LINE__+50000)
		goto true;
		}
	/*
	 *	and.l	#65535,Dx	=>	moveq	#0,Dy
	 *	move.l	Dx,Dy			move.w	Dx,Dy
	 *
	 * Where Dx is dead.
	 */
	if(OP_AND(op1) && sm1==IMM && dm1==REG && ISD(dr1) && 
	   i1->src.disp==65535 && i1->flags==LENL &&
	   op2==MOVE && i2->src.amode==REG && i2->src.areg==dr1 && 
	   i2->dst.amode==REG && ISD(i2->dst.areg) && 
	   !(i2->live & RM(dr1)) && i2->flags==LENL)
		{
		setupinst(i1,MOVEQ,0,IMM,0,REG,i2->dst.areg);
		i1->live|=RM(i2->dst.areg);
		i2->flags=LENW;

		DBG(p2_line=__LINE__+50000)
		goto true;
		}
	/*
	 *	ext.l	Dx		=>	<deleted>
	 *	move.w	Dx,X			move.w	Dx,X
	 *
	 * Where Dx is dead.
	 */
	if(op1==EXT && i1->flags==LENL && 
	   op2==MOVE && i2->src.amode==REG && i2->src.areg==i1->src.areg && 
	   i2->flags==LENW && !(i2->flags & RM(i1->src.areg)))
		{
		delinst(i1);

		DBG(p2_line=__LINE__+50000)
		goto true2;
		}
	/*.
	 *	move.l	#N,Dx		=>	move.l	#N,Dx
	 *	...(1)				...(1)
	 *	move.l	#M,Dx			<deleted>
	 *	move.wb	Y,Dx			move.wb	Y,Dx
	 *
	 * Where Dx is not set in (1) or only lower word is set in (1) 
	 * and N,M are cleared by the moves.
	 */
/* that rule makes trouble beacause it uses half registers and POPT was
   not designed to take that into accound (live/death analysis become fals
   after that instruction...)

	if(OP_MOVE(op1) && sm1==IMM && dm1==REG && ISD(dr1) && 
	   !(i1->flags&(LENW|LENB)))
		{INST *ti=i2;int sizeoftouch=0;
		while(1)
			{
			if(breakflow(ti)) goto end36;
			if(ti->opcode==MOVE && ti->dst.amode==REG && 
			   ti->dst.areg==dr1)
			   	{int i=ti->flags?ti->flags:4;
			   	if(i>sizeoftouch) sizeoftouch=i;
			   	}
			else if(sets(ti,dr1)) break;
			ifn(ti=ti->next) goto end36;
			}
		if(OP_MOVE(ti->opcode) && ti->src.amode==IMM && 
		   ti->dst.amode==REG && ti->dst.areg==dr1 && 
		   !(ti->flags&(LENW|LENB)) && (ti2=ti->next) && 
		   OP_MOVE(ti2->opcode) && ti2->dst.amode==REG && 
		   ti2->dst.areg==dr1 && (ti2->flags&(LENW|LENB)) && 
		   (ti2->flags>=sizeoftouch) && 
		   !(ti->src.disp&~(ti2->flags==LENW?65535:255)))
			{
			/*.
			 * mark Dx as beeing still used
			 * /
			ti2 = i2;
			while(ti2!=ti)
				{
				ti2->live = ti2->refs = ti2->sets = -1;
				ti2 = ti2->next;
				}
			delinst(ti);
			DBG(p2_line=__LINE__+50000)
			goto true;
			}
	   	}
end36:	/*
	 *	move.l	Dx,Dy		=>	<deleted>
	 *	....				....
	 *	<stuff1>			<stuff2>
	 *	....				....
	 *	<inst that sets Dx>(2)		<inst that sets Dx>
	 *
	 * Where Dx is dead, Dy is dead at (2). <stuff2> is <stuff1> 
	 * where Dy is replaced by Dx. UNSAFE: the deleted move may
	 * set flags. Note (2) may also be a breakflow instruction.
	 */
	if((!safe || cc_modified(i2)) && 
	   OP_MOVE(op1) && sm1==REG && ISD(i1->src.areg) && dm1==REG && 
	   ISD(dr1) && i1->flags==LENL && !(i1->live&RM(i1->src.areg)))
		{
		register INST *end;int Dx=i1->src.areg;
		/*.
		 *	find end of stuff
		 */
		end=i2;
		while(1)
			{
			if(sets(end,dr1) && 
			  (end->opcode==JSR || end->opcode==BSR)) goto end37;
			if(sets(end,Dx) || breakflow(end)) 
				{
				if(end->live & RM(dr1)) goto end37;
				else break;
				}
			ifn(end=end->next) goto end37;
			}
		/*.
		 *	replace Dx by Dy
		 */
		ti2 = i2;end=end->next;
		while(ti2 != end)
			{int m;
			ti2->live = ti2->refs = ti2->sets = -1;
			m=M(ti2->src.amode);
			if(m==REG && ti2->src.areg==dr1) ti2->src.areg=Dx;
			if(m==PCDX && ti2->src.ireg==dr1) ti2->src.ireg=Dx;
			if(m==REGIDX && ti2->src.ireg==dr1) ti2->src.ireg=Dx;
			m=M(ti2->dst.amode);
			if(m==REG && ti2->dst.areg==dr1) ti2->dst.areg=Dx;
			if(m==PCDX && ti2->dst.ireg==dr1) ti2->dst.ireg=Dx;
			if(m==REGIDX && ti2->dst.ireg==dr1) ti2->dst.ireg=Dx;
			ti2=ti2->next;
			}
	    	delinst(i1);
	    	DBG(p2_line=__LINE__+50000)
	    	goto true2;
		}
end37:	/*
	 *	move.z	Ry,Rx		=>	<deleted>
	 *	...(1)				...(1)
	 *	INST.x ..M(Am,Rx.y)..		INST.x ..M(An,Ry.y)..
	 *
	 * Where Rx is dead and not used in (1) and Ry not set in (1). 
	 * Note, z must be greater than y.
	 */
	if(op1==MOVE && sm1==REG && dm1==REG)
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

		if(ti2->src.amode==(REGIDX|XLONG) && ti2->src.ireg==dr1 &&
		   i1->flags!=LENL) goto end1;
		if(ti2->src.amode==REGIDX && ti2->src.ireg==dr1 &&
		   i1->flags==LENB) goto end1;
		if(ti2->dst.amode==(REGIDX|XLONG) && ti2->dst.ireg==dr1 &&
		   i1->flags!=LENL) goto end1;
		if(ti2->dst.amode==REGIDX && ti2->dst.ireg==dr1 &&
		   i1->flags==LENB) goto end1;

		flg=0;

		m=ti2->src.amode&~XLONG;
		if(m==REGIDX && ti2->src.ireg==dr1)
			{
			ti2->src.ireg=sr1;
			flg=1;
			}
		m=ti2->dst.amode&~XLONG;
		if(m==REGIDX && ti2->dst.ireg==dr1)
			{
			ti2->dst.ireg=sr1;
			flg=1;
			}
		if(flg)	{
		    	delinst(i1);
		    	DBG(p2_line=__LINE__+50000)
		    	goto true2;
		    	}
	   	}
end1:	/*
	 *	sub[q].z #N,Rx		=>	<deleted>
	 *	...(1)				...(1)
	 *	INST	..[M](Am,Rx.y)..	INST	..[M]-N(Am,Rx.y)..
	 *
	 * Where Rx is dead and not used in (1). If Rx==Dx then .z 
	 * must be .l. Note: if Rx uses a multiplier then multiply N too.
	 */
	if(OP_SUB(op1) && sm1==IMM && dm1==REG && D8OK(i1->src.disp))
	   	{int m;bool flg;int sh1,sh2;
	   	if(ISD(dr1) && i1->flags!=LENL) goto end2;
		ti2 = i2;
		while (!uses(ti2, dr1)) 
			{
			if  (breakflow(ti2))     goto end2;
			ifn (ti2=ti2->next)	 goto end2;
			}
		if(ti2->label) goto end2;
		if(ti2->live & RM(dr1)) goto end2;
		
		/*.
		 * chk disp 
		 */
		m=ti2->src.amode&~(XLONG|X2|X4|X8);
		if(m==REGIDX && ti2->src.ireg==dr1)
			{
			m = ti2->src.amode;
			sh1 = (m & X2)?1:(m & X4)?2:(m & X8)?3:0;
			m = ti2->src.disp-(i1->src.disp<<sh1);
			ifn(D8OK(m)) goto end2;
			}
		m=ti2->dst.amode&~(XLONG|X2|X4|X8);
		if(m==REGIDX && ti2->dst.ireg==dr1)
			{
			m = ti2->dst.amode;
			sh2 = (m & X2)?1:(m & X4)?2:(m & X8)?3:0;
			m = ti2->dst.disp-(i1->src.disp<<sh2);
			ifn(D8OK(m)) goto end2;
			}

		flg=0;

		m=ti2->src.amode&~(XLONG|X2|X4|X8);
		if(m==REGIDX && ti2->src.ireg==dr1)
			{
			ti2->src.disp -= (i1->src.disp<<sh1);
			flg=1;
			}
		m=ti2->dst.amode&~(XLONG|X2|X4|X8);
		if(m==REGIDX && ti2->dst.ireg==dr1)
			{
			ti2->dst.disp -= (i1->src.disp<<sh2);
			flg=1;
			}
		if(flg)	{
		    	delinst(i1);
		    	DBG(p2_line=__LINE__+50000)
		    	goto true2;
		    	}
	   	}
end2:	/*
	 *	add[q].z #N,Rx		=>	<deleted>
	 *	...(1)				...(1)
	 *	INST	..[M](Am,Rx.y)..	INST	..[M+]N(Am,Rx.y)..
	 *
	 * Where Rx is dead and not used in (1). If Rx==Dx then .z 
	 * must be .l. Note: if Rx uses a multiplier then multiply N too.
	 */
	if(OP_ADD(op1) && sm1==IMM && dm1==REG && D8OK(i1->src.disp))
	   	{int m;bool flg;int sh1,sh2;
	   	if(ISD(dr1) && i1->flags!=LENL) goto end3;
		ti2 = i2;
		while (!uses(ti2, dr1)) 
			{
			if  (breakflow(ti2))     goto end3;
			ifn (ti2=ti2->next)	 goto end3;
			}
		if(ti2->label) goto end3;
		if(ti2->live & RM(dr1)) goto end3;
		
		/*.
		 * chk disp 
		 */
		m=ti2->src.amode&~(XLONG|X2|X4|X8);
		if(m==REGIDX && ti2->src.ireg==dr1)
			{
			m = ti2->src.amode;
			sh1 = (m & X2)?1:(m & X4)?2:(m & X8)?3:0;
			m = ti2->src.disp+(i1->src.disp<<sh1);
			ifn(D8OK(m)) goto end3;
			}
		m=ti2->dst.amode&~(XLONG|X2|X4|X8);
		if(m==REGIDX && ti2->dst.ireg==dr1)
			{
			m = ti2->dst.amode;
			sh2 = (m & X2)?1:(m & X4)?2:(m & X8)?3:0;
			m = ti2->dst.disp+(i1->src.disp<<sh2);
			ifn(D8OK(m)) goto end3;
			}

		flg=0;

		m=ti2->src.amode&~(XLONG|X2|X4|X8);
		if(m==REGIDX && ti2->src.ireg==dr1)
			{
			ti2->src.disp += (i1->src.disp<<sh1);
			flg=1;
			}
		m=ti2->dst.amode&~(XLONG|X2|X4|X8);
		if(m==REGIDX && ti2->dst.ireg==dr1)
			{
			ti2->dst.disp += (i1->src.disp<<sh2);
			flg=1;
			}
		if(flg)	{
		    	delinst(i1);
		    	DBG(p2_line=__LINE__+50000)
		    	goto true2;
		    	}
	   	}
end3:	/*
	 *	move.x	Rm,X		=>	move.x	Rm,X
	 *	...(1)				...
	 *	INST.x	X,Y			INST.x	Rm,Y
	 *
	 * Where 'x' is the same, and 'X' has no side-effects and 
	 * is not register, Rm is not set in (1). INST!=lea
	 */
	if (OP_MOVE(op1) && (sm1 == REG) && (dm1!=REG) && !(dm1 & (DEC|INC)))
		{
		ti2 = i2;
		while(!(ti2->flags == i1->flags && opeq(&i1->dst,&ti2->src)))
			{
__3__:			if  (ubreakflow(ti2))		goto end4;
			if  (ti2->opcode == BSR)	goto end4;
			if  (ti2->opcode == JSR)	goto end4;
			if  (ti2->sets & RM(sr1))	goto end4; /* oops I've forgot it: 03/04/96 */
			ifn (ti2=ti2->next)		goto end4;
			}
		if(ti2->label)				goto end4;
		if(ti2->dst.amode == NONE)		goto end4;
		if(ti2->opcode == LEA || 
		   ti2->opcode == MOVEQ) goto __3__;

		if(ti2->opcode==ADDQ || ti2->opcode==ADDI) ti2->opcode=ADD;
		else if(ti2->opcode==SUBQ || ti2->opcode==SUBI) ti2->opcode=SUB;

		freeop(&ti2->src);
		ti2->src   = i1->src;
		ti2->refs |= RM(sr1);
		do
			{
			ti2=ti2->prev;
			ti2->live |= RM(sr1);
			}
		while(ti2!=i1);
	   	DBG(p2_line=__LINE__+50000)
	    	goto true;
		}
end4:	/*
	 *	move.x	X,Rm		=>	move.x	X,Rm
	 *	...(1)				...
	 *	INST.x	X,Y			INST.x	Rm,Y
	 *
	 * Where 'x' is the same, and 'X' has no side-effects and 
	 * is not register, Rm is not set in (1).
	 */
	if (OP_MOVE(op1) && (dm1 == REG) && (sm1!=REG) && 
	    !(sm1 & (DEC|INC))) 
		{
		ti2 = i2;
		while(!(ti2->flags == i1->flags && opeq(&i1->src,&ti2->src)))
			{
__4__:			if  (ubreakflow(ti2))		goto end5;
			if  (ti2->opcode == BSR)	goto end5;
			if  (ti2->opcode == JSR)	goto end5;
			if  (ti2->sets & RM(dr1))	goto end5; /* oops I've forgot it: 03/04/96 */
			ifn (ti2=ti2->next)		goto end5;
			}
		if(ti2->label)				goto end5;
		if(ti2->dst.amode == NONE)		goto end5;
		if(ti2->opcode == LEA || 
		   ti2->opcode == MOVEQ) goto __4__;

		if(ti2->opcode==ADDQ || ti2->opcode==ADDI) ti2->opcode=ADD;
		else if(ti2->opcode==SUBQ || ti2->opcode==SUBI) ti2->opcode=SUB;

		freeop(&ti2->src);
		ti2->src   = i1->dst;
		ti2->refs |= RM(dr1);
		do
			{
			ti2=ti2->prev;
			ti2->live |= RM(dr1);
			}
		while(ti2!=i1);
	   	DBG(p2_line=__LINE__+50000)
	    	goto true;
		}
end5:	/*.
	 *	moveq	#N,Dn		=>	move.l	X,Dn
	 *	add.l	X,Dn			addq.l	#N,Dn
	 *
	 * That helps some future optimizations.
	 */
	if(op1==MOVEQ && i1->src.disp>=-8 && i1->src.disp<=8 &&
	   op2==ADD && i2->flags==LENL && dm2==REG && dr2==dr1)
		{int n = i1->src.disp;
		i1->opcode    = MOVE;
		i1->flags     = LENL;
		i1->src       = i2->src;
		setupinst(i2,ADD,LENL,IMM,n,REG,dr1);
		i1->refs   =
		i1->sets   =
		i1->live   = -1;
	   	DBG(p2_line=__LINE__+50000)
		goto true;
		}
next:	return NULL;
true:	return i1->next;
true2:	return i2;
	}

/*.
 * try to swap instruction 
 */
void try_swap()
	{
	register INST *ip,*ip2;

	for(ip=head, ip2=ip->next; ip && ip2; ip=ip2, ip2=ip->next) 
	if(!ip2->label)
		{
		int reg;
		int sm1,sm2,op1,op2;
		struct opnd op;

		op1 = ip->opcode;
		op2 = ip2->opcode;
		sm1 = ip->src.amode;
		sm2 = ip2->src.amode;
		/*.
		 * 	MOVE.x	X,Reg	=>	MOVE.x	Y,Reg
		 *	INST1.x	Y,Reg		INST1.x	X,Reg
		 *
		 * X and Y have no side-effect. INST=ADD | AND | OR | EOR
		 */
		ifn(ip->flags == ip2->flags) continue;
		ifn((op1==MOVE || op1==MOVEA) && ip->dst.amode==REG) continue;
		ifn(((OP_ADD(op2) && op2!=ADDQ) || OP_AND(op2) || OP_OR(op2) || OP_EOR(op2)) &&
		     ip2->dst.amode==REG && ip2->dst.areg==(reg=ip->dst.areg))
			continue;

		if((sm1&(DEC|INC)) || (sm2&(DEC|INC))) continue; /* could be improved */
		if(sm1==REGIDX && ip->src.areg==reg) break;
		if(sm2==REGIDX && ip2->src.areg==reg) break;

		op       = ip->src;
		ip->src  = ip2->src;
		ip2->src = op;
		}
	peep();
	}
