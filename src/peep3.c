/*.
 *	peep3.c - 3-instruction peephole optimizations
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

int p3_line;

/*.
 * ipeep3(ip) - look for 3-instruction optimizations at the given inst.
 */
static INST *ipeep3(i1)
	INST	*i1;
	{
	INST	*i2;	/* the next instruction */
	INST	*i3;	/* the third instruction */

	int	op1;
	int	op2;
	int	op3;
	
	int	sm1,dm1,sr1,dr1;
	int	sm2,dm2,sr2,dr2;
	int	sm3,dm3,sr3,dr3;

	ifn(i2 = i1->next) return NULL;
	ifn(i3 = i2->next) return NULL;
	if(i2->label || i3->label) return NULL;

	op1 = i1->opcode; sm1 = i1->src.amode; sr1 = i1->src.areg; 
	dm1 = i1->dst.amode; dr1 = i1->dst.areg;

	op2 = i2->opcode; sm2 = i2->src.amode; sr2 = i2->src.areg; 
	dm2 = i2->dst.amode; dr2 = i2->dst.areg;

	op3 = i3->opcode; sm3 = i3->src.amode; sr3 = i3->src.areg; 
	dm3 = i3->dst.amode; dr3 = i3->dst.areg;

	/*.
	 *	swap	Dn		=>	and.l	#65535,Dn
	 *	clr.w	Dn
	 *	swap	Dn
	 *
	 * for internal purpose
	 */
	if(op1==SWAP && op2==CLR && op3==SWAP && i2->flags==LENW &&
	   sr1==sr2 && sr2==sr3)
		{
		setupinst(i1,AND,LENL,IMM,65535,REG,sr1);
		delinst(i2);--s_idel;
		delinst(i3);--s_idel;
		return ipeep3bis(i1->next);
		}
	/*
	 *	move.l	Am,Rn		=>	lea	N(Am),Ao
	 *	add.l	#N,Rn			<deleted>
	 *	move.l	Rn,Ao			<deleted>
	 *
	 * Also, Rn must be dead after the third instruction.
	 * UNSAFE since the add sets the flags and move don't so check 
	 * if the 4th inst set them.
	 */
	if ((!safe || cc_modified(i3->next) || ISA(dr1)) &&
	    OP_MOVE(op1) && (i1->flags == LENL) &&
	    (sm1 == REG) && ISA(sr1) && (dm1 == REG)) 
		{
		if (OP_ADD(op2) && (i2->flags & LENL) &&
		    (sm2 == IMM) && DOK(i2->src.disp) &&
		    (dm2 == REG) && (dr2 == dr1)) 
			{
			if (OP_MOVE(op3) && (i3->flags & LENL) &&
			    (sm3 == REG) && (sr3 == dr1) &&
			    (dm3 == REG) && ISA(dr3) &&
			    !(i3->live & RM(sr3))) 
				{
				/*.
				 * rewrite i1 and delete i2 and i3
				 */
				setupinst(i1,LEA,0,REGID,i2->src.disp,sr1,
						   REG,dr3); 
			    	delinst(i2);
			    	delinst(i3);
			    	DBG(p3_line=__LINE__)
			    	goto true;
				}
			}
		}
	/*
	 *	move.l	Rm,Rn		=>	move.l	Rm,Ao
	 *	add.l	#N,Rn			lea	N(Ao),Ao
	 *	move.l	Rn,Ao			<deleted>
	 *
	 * Also, Rn must be dead after the third instruction.
	 * UNSAFE since the add sets the flags (if Rn==Dn) and move don't 
	 * so we check if the 4th inst set them. 
	 */
	if ((!safe || cc_modified(i3->next) || ISA(dr1)) &&
	    OP_MOVE(op1) && (i1->flags==LENL) &&
	    (sm1 == REG) && (dm1 == REG)) 
		{
		if (OP_ADD(op2) && (i2->flags==LENL) &&
		    (sm2 == IMM) && DOK(i2->src.disp) &&
		    (dm2 == REG) && (dr2 == dr1)) 
			{
			if (OP_MOVE(op3) && (i3->flags & LENL) &&
			    (sm3 == REG) && (sr3 == dr1) &&
			    (dm3 == REG) && ISA(dr3) &&
			    !(i3->live & RM(sr3)))
				{
				/*.
				 * rewrite i1 and i2 and delete i3
				 */
				i1->dst.areg = dr3;
				i1->live    |= RM(dr3);
				setupinst(i2,LEA,0,REGID,i2->src.areg,dr3,
						   REG,dr3);
			    	delinst(i3);
			    	DBG(p3_line=__LINE__)
			    	goto true;
				}
			}
		}
	/*
	 *	move.l	Am,Rn		=>	lea	-N(Am),Ao
	 *	sub.l	#N,Rn			<deleted>
	 *	move.l	Rn,Ao			<deleted>
	 *
	 * Also, Rn must be dead after the third instruction.
	 * UNSAFE since the add sets the flags and move don't so check 
	 * if the 4th inst set them.
	 */
	if ((!safe || cc_modified(i3->next) || ISA(dr1)) &&
	    OP_MOVE(op1) && (i1->flags==LENL) &&
	    (sm1 == REG) && ISA(sr1) && (dm1 == REG)) 
		{
		if (OP_SUB(op2) && (i2->flags==LENL) &&
		    (sm2 == IMM) && DOK(i2->src.disp) &&
		    (dm2 == REG) && (dr2 == dr1)) 
			{
			if (OP_MOVE(op3) && (i3->flags==LENL) &&
			    (sm3 == REG) && (sr3 == dr1) &&
			    (dm3 == REG) && ISA(dr3) &&
			    !(i3->live & RM(sr3))) 
				{
				/*.
				 * rewrite i1 and delete i2 and i3
				 */
				setupinst(i1,LEA,0,REGID,-i2->src.disp,sr1,
						   REG,dr3); 
			    	delinst(i2);
			    	delinst(i3);
			    	DBG(p3_line=__LINE__)
			    	goto true;
				}
			}
		}
	/*
	 *	move.l	Am,Rn		=>	lea	0(Am,Ro),Ap
	 *	add.x	Ro,Rn			<deleted>
	 *	move.l	Rn,Ap			<deleted>
	 *
	 * The second instruction can be either a word or long add.
	 * Also, Rn must be dead after the third instruction.
	 * UNSAFE since the add sets the flags (if Rn==Dn) and move 
	 * don't so check if the 4th inst set them. Not for 68040/60.
	 */
	if (!mc60 && !mc40 && (!safe || cc_modified(i3->next) || ISA(dr2)) &&
	    OP_MOVE(op1) && (i1->flags==LENL) &&
	    (sm1 == REG) && ISA(sr1) && (dm1 == REG))
		{
		if (OP_ADD(op2) && (i2->flags & (LENL|LENW)) &&
		    (sm2 == REG) && (dr1 != sr2) &&
		    (dm2 == REG) && (dr2 == dr1))
			{
			if (OP_MOVE(op3) && (i3->flags & LENL) &&
			    (sm3 == REG) && (sr3 == dr1) &&
			    (dm3 == REG) && ISA(dr3) &&
			    !(i3->live & RM(sr3)))
				{
				/*.
				 * rewrite i1 and delete i2 and i3
				 */
				setupinst(i1,LEA,0,
				((i2->flags & LENL)?(REGIDX|XLONG):REGIDX),
				0,sr1,sr2,REG,dr3);
			    	delinst(i2);
			    	delinst(i3);
			    	DBG(p3_line=__LINE__)
			    	goto true;
				}
			}
		}
	/*
	 *	move.l	X(Am),Rn	=>	move.l	X(Am),Ao
	 *	add.l	#N,Rn			<deleted>
	 *	move.l	Rn,Ao			lea	N(Ao),Ao
	 *
	 * Also, Rn must be dead after the third instruction.
	 * UNSAFE since the add sets the flags and move don't so check 
	 * if the 4th inst set them.
	 */
	if ((!safe || cc_modified(i3->next)) &&
	    OP_MOVE(op1) && (i1->flags==LENL) &&
	    (sm1 == REGI || sm1 == REGID) && (dm1 == REG))
		{
		if (OP_ADD(op2) && (i2->flags==LENL) &&
		    (sm2 == IMM) && DOK(i2->src.disp) &&
		    (dm2 == REG) && (dr2 == dr1))
			{
			if (OP_ADD(op3) && (i3->flags & LENL) &&
			    (sm3 == REG) && (sr3 == dr1) &&
			    (dm3 == REG) && ISA(dr3) &&
			    !(i3->live & RM(sr3)))
				{
				/*.
				 * rewrite i1 and i3 and delete i2
				 */
				i1->dst.areg = dr3;
				i1->live    |= RM(dr3);

				setupinst(i3,LEA,0,REGID,i2->src.disp,dr3,
						   REG,dr3);
			    	delinst(i2);
			    	DBG(p3_line=__LINE__)
			    	goto true;
				}
			}
		}
	/*
	 *	move.x	X,Dn		=>	move.x	X,Do
	 *	ext.y	Dn			ext.y	Do
	 *	move.y	Dn,Do		=>	<deleted>
	 *
	 * Where Dn is dead.
	 */
	if (OP_MOVE(op1)&&(op2 == EXT)&&OP_MOVE(op3)&&
	    (dm1 == REG) && ISD(dr1) && (sm2 == REG) && (sm3 == REG) &&
	    (dm3 == REG) && ISD(dr3) && (dr1 == sr2) && (dr1 == sr3) &&
	    (i2->flags == i3->flags))
		{
		ifn (i3->live & RM(sr3))
			{
			i1->dst.areg = dr3;
			i1->refs = i1->sets = i1->live = -1;
			i2->src.areg = dr3;
			i2->refs = i2->sets = i2->live = -1;
		    	delinst(i3);
		    	DBG(p3_line=__LINE__)
		    	goto true;
			}
		}
	/*
	 *	move.l	X,Dm		=>	move.l	X,An
	 *	INST				INST
	 *	move.l	Dm,An		=>	<deleted>
	 *
	 * Where INST doesn't modify Dm, and Dm is dead after i3
	 * UNSAFE since the last move doesn't change the flags.
	 */
	if ((!safe || cc_modified(i3->next)) &&
	    OP_MOVE(op1) && OP_MOVE(op3) && (dm1 == REG) && ISD(dr1) && 
	    (sm3 == REG) && (dr1 == sr3) && (dm3 == REG) && ISA(dr3) &&

/* ADDED BY TETISOFT
 * Avoid move.l (a0),d0    =>    move.l (a0),a0
 *	 addq.l #1,(a0)          addq.l #1,(a0)
 *	 move.l d0,a0
 */
	    !((sm1 == REGI) && (uses(i2, sr1)) && (sr1 == dr3)) &&
	    !uses(i2, sr3)) 
		{
		ifn (i3->live & sr3)
			{
			i1->dst.areg = dr3;
			i1->refs = i1->sets = i1->live = -1;
			delinst(i3);
		    	DBG(p3_line=__LINE__)
			goto true;
			}
		}
	return ipeep3bis(i1);
true:	return i1->next;
true2:	return i2;
true3:	return i3;
}

/*.
 * peep3() - peephole optimizations with a window size of 3
 */
bool peep3()
	{
	register INST	*ip,*ni;
	register bool	changed = FALSE;

	DBG(printf("peep (3) : "))
	for(ip=head; ip ;)
		{int line=ip->lineno;
		if(ni = ipeep3(ip))
			{
			++s_peep3;
			ip = ni;
			changed = TRUE;
			DBG(printf("%d(%d) ",p3_line,line);fflush(stdout))
			}
		else	ip=ip->next;
		}
	DBG(printf("\n"); fflush(stdout))
	return changed;
	}
