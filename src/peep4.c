/*.
 *	peep4.c - 4-instruction peephole optimizations
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

static	int	p4_line;

/*.
 * ipeep4(ip) - look for 4-instruction optimizations at the given inst.
 */
static INST *ipeep4(i1)
	INST	*i1;
	{
	INST	*i2;	/* the next instruction */
	INST	*i3;	/* the third instruction */
	INST	*i4;	/* the 4th instr. */

	int	op1;
	int	op2;
	int	op3;
	int	op4;
	
	int	sm1,dm1,sr1,dr1;
	int	sm2,dm2,sr2,dr2;
	int	sm3,dm3,sr3,dr3;
	int	sm4,dm4,sr4,dr4;

	ifn(i2 = i1->next) return NULL;
	ifn(i3 = i2->next) return NULL;
	ifn(i4 = i3->next) return NULL;
	if(i2->label || i3->label || i4->label) return NULL;

	op1 = i1->opcode; sm1 = i1->src.amode; sr1 = i1->src.areg; 
	dm1 = i1->dst.amode; dr1 = i1->dst.areg;

	op2 = i2->opcode; sm2 = i2->src.amode; sr2 = i2->src.areg; 
	dm2 = i2->dst.amode; dr2 = i2->dst.areg;

	op3 = i3->opcode; sm3 = i3->src.amode; sr3 = i3->src.areg; 
	dm3 = i3->dst.amode; dr3 = i3->dst.areg;

	op4 = i4->opcode; sm4 = i4->src.amode; sr4 = i4->src.areg; 
	dm4 = i4->dst.amode; dr4 = i4->dst.areg;

	/*
	 *	move.w	X,Dx		=>	moveq	#0,Dx
	 *	swap	Dx			move.w	X,Dx
	 *	clr.w	Dx			<deleted>
	 *	swap	Dx			<deleted>
	 */
	if(OP_MOVE(op1) && dm1==REG && i1->flags==LENW &&
	   op2==SWAP && sm2==REG && sr2==dr1 &&
	   op3==CLR && sm3==REG && sr3==dr1 && i3->flags==LENW &&
	   op4==SWAP && sm4==REG && sr4==dr1)
		{
		setupinst(inspreinst(i1),MOVEQ,0,IMM,0,REG,dr1);
		delinst(i2);
		delinst(i3);
		delinst(i4);
		
		DBG(p4_line=__LINE__)
		goto true;
		}
	/*
	 *	move.x	X,Dx		=>	moveq	#0,Dx
	 *	swap	Dx			move.y	X,Dx
	 *	clr.w	Dx			<deleted>
	 *	swap	Dx			<deleted>
	 *
	 * Where X is REG or IMM. If x=.l then y=.w else y=x
	 */
	if(OP_MOVE(op1) && (sm1==IMM|sm1==REG) && dm1==REG &&
	   op2==SWAP && sm2==REG && sr2==dr1 &&
	   op3==CLR && sm3==REG && sr3==dr1 && i3->flags==LENW &&
	   op4==SWAP && sm4==REG && sr4==dr1)
		{
		setupinst(inspreinst(i1),MOVEQ,0,IMM,0,REG,dr1);
		if(i1->flags==LENL) i1->flags=LENW;
		delinst(i2);
		delinst(i3);
		delinst(i4);
		
		DBG(p4_line=__LINE__)
		goto true;
		}
	/*
	 *	swap	Dx		=>	moveq	#0,Dy
	 *	clr.w	Dx			move.w	Dx,Dy
	 *	swap	Dx			<deleted>
	 *	move.l	Dx,Dy			<deleted>
	 *
	 * Where Dx is dead.
	 */
	if(op1==SWAP && sm1==REG && ISD(sr1) &&
	   op2==CLR  && sm2==REG && sr2==sr1 &&
	   op3==SWAP && sm3==REG && sr3==sr1 &&
	   op4==MOVE && sm4==REG && sr4==sr1 && dm4==REG && ISD(dr4) && 
	   i4->flags==LENW && !(i4->live&RM(sr1)))
		{
		setupinst(i1,MOVEQ,0,IMM,0,REG,dr4);i1->live|=RM(dr4);
		setupinst(i2,MOVE,LENW,REG,sr1,REG,dr4);
		delinst(i3);
		delinst(i4);

		DBG(p4_line=__LINE__)
		goto true;
		}
	/*
	 *	move.w	Dm, Dn		=>	dbf	Dm,lbl
	 *	sub.w	#1,Dm			<deleted>
	 *	tst.w	Dn			<deleted>
	 *	bne	lbl			<deleted>
	 *
	 * Where Dn is dead after the test.
	 */
	if(i1->flags==i2->flags && i2->flags==i3->flags && i3->flags==LENW &&
	   op1==MOVE && OP_SUB(op2) && op3==TST && op4==BNE &&
	   sm1 == REG && ISD(sr1) && dm1 == REG && ISD(dr1) && 
	   sm2 == IMM && i2->src.disp==1 && dm2==REG && dr2==sr1 && 
	   sm3 == REG && sr3==dr1 && !(i3->live & RM(dr1)))
	   	{
	   	setupinst(i1,DBF,0,REG,sr1,ABS|SYMB,strsave(i4->src.astr));
	   	delinst(i4);
	   	delinst(i3);
	   	delinst(i2);
	    	DBG(p4_line=__LINE__)
	    	goto true;
	   	}
	/*
	 *	move.l	Dm,Dn		=>		dbf	Dm,lbl
	 *	sub.l	#1,Dm				clr.w	Dm
	 *	tst.l	Dn				subq.l	#1,Dm
	 *	bne	lbl				bcc	lbl
	 *	...					...
	 *
	 * Where Dn is dead after the test.
	 * This is faster since the inner loop is faster (if dm!=0).
	 */
	if(!size && 
	   i1->flags==i2->flags && i2->flags==i3->flags && i3->flags==LENL &&
	   op1==MOVE && OP_SUB(op2) && op3==TST && op4==BNE && 
	   sm1 == REG && ISD(sr1) && dm1 == REG && ISD(dr1) && 
	   sm2 == IMM && i2->src.disp==1 && dm2==REG && dr2==sr1 && 
	   sm3 == REG && sr3==dr1 && !(i3->live & RM(dr1)))
	   	{ 
		INST *ni;
		char *lab0=i4->src.astr;		/* lbl 	 */

	   	ni = i1;
/* old move */	setupinst(ni,DBF,0,REG,sr1,ABS|SYMB,strsave(lab0))->live=-1;ni = ni->next;
/* old sub */	setupinst(ni,CLR,LENW,REG,sr1,NONE)->live=-1;ni = ni->next;
/* old tst */	setupinst(ni,SUBQ,LENL,IMM,1,REG,sr1)->live=-1;ni = ni->next;
/* old bne */	setupinst(ni,BCC,0,ABS|SYMB,lab0,NONE)->live=-1;

	    	DBG(p4_line=__LINE__)
	    	goto true;
	   	}
	return NULL;
true:	return i1->next;
true2:	return i2;
true3:	return i3;
}

/*.
 * peep4() - peephole optimizations with a window size of 3
 */
bool peep4()
	{
	register INST	*ip,*ni;
	register bool	changed = FALSE;

	DBG(printf("peep (4) : "))
	for(ip=head; ip ;)
		{int line=ip->lineno;
		if(ni = ipeep4(ip))
			{
			++s_peep4;
			ip = ni;
			changed = TRUE;
			DBG(printf("%d(%d) ",p4_line,line);fflush(stdout))
			}
		else	ip=ip->next;
		}
	DBG(printf("\n"); fflush(stdout))
	return changed;
	}
