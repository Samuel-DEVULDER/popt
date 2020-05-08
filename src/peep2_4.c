/*.
 *	peep2_4.c - 2-instruction peephole optimizations (following)
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

extern int p2_line;

/*.
 * ipeep2_4(i1) - look for 2-instruction optimizations at the given 
 *		  inst. Part IV.
 */
INST *ipeep2_4(i1)
	register INST	*i1;
	{
	register INST	*i2;		/* the next instruction */
	register INST	*ti2;		/* "temporary" next inst */

	register int	op1, op2;	/* opcodes, for speed */
	register int	sm1, dm1;	/* src, dst amode inst. 1 */
	register int	dr1;		/* dest. reg. inst. 1 */

	ifn(i2 = i1->next) goto next;

	op1 = i1->opcode;

	sm1 = i1->src.amode;
	dm1 = i1->dst.amode;
	dr1 = i1->dst.areg;
	
	/*
	 *	INST.b	..(Am)..	=>	INST.b	..(Am)+..
	 *	....				....
	 *	<stuff>				<stuff>
	 *	....				....
	 *	add[q].lw #1,Am		=>	<deleted>
	 *
	 * Nothing in "stuff" can refer to Am.
	 * Note: we go upward..
	 */
	if (OP_ADD(op1) && (i1->flags & (LENL|LENW)) &&
	    (sm1 == IMM) && (i1->src.disp == 1) && (dm1 == REG) && ISA(dr1))
		{
		if(i1->label) goto end30;
		for(ti2=i1->prev;ti2;ti2=ti2->prev)
			{
			if(ti2->src.amode == REGI && ti2->src.areg == dr1)
				{
				if (ti2->flags!=LENB) goto end30;
				ti2->src.amode |= INC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+40000)
			    	goto true2;
				}
			if(ti2->dst.amode == REGI && ti2->dst.areg == dr1)
				{
				if (ti2->flags!=LENB) goto end30;
				ti2->dst.amode |= INC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+40000)
			    	goto true2;
				}
			if(uses(ti2, dr1)) goto end30;
			if(breakflow(ti2)) goto end30;
			}
		}
end30:	/*
	 *	INST.w	..(Am)..	=>	INST.w	..(Am)+..
	 *	....				....
	 *	<stuff>				<stuff>
	 *	....				....
	 *	add[q].lw #2,Am		=>	<deleted>
	 *
	 * Nothing in "stuff" can refer to Am.
	 * NOTE: we go upward...
	 */
	if (OP_ADD(op1) && (i1->flags & (LENL|LENW)) &&
	    (sm1 == IMM) && (i1->src.disp == 2) && (dm1 == REG) && ISA(dr1))
		{
		if(i1->label) goto end31;
		for(ti2=i1->prev;ti2;ti2=ti2->prev)
			{
			if(ti2->src.amode == REGI && ti2->src.areg == dr1)
				{
				if(ti2->flags!=LENW) goto end31;
				ti2->src.amode |= INC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+40000)
			    	goto true2;
				}
			if(ti2->dst.amode == REGI && ti2->dst.areg == dr1)
				{
				if(ti2->flags!=LENW) goto end31;
				ti2->dst.amode |= INC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+40000)
			    	goto true2;
				}
			if(uses(ti2, dr1)) goto end31;
			if(breakflow(ti2)) goto end31;
			}
		}
end31:	/*
	 *	INST.l	..(Am)..	=>	INST.l	..(Am)+..
	 *	....				....
	 *	<stuff>				<stuff>
	 *	....				....
	 *	add[q].lw #4,Am		=>	<deleted>
	 *
	 * Nothing in "stuff" can refer to Am.
	 * NOTE: we go upward...
	 */
	if (OP_ADD(op1) && (i1->flags & (LENL|LENW)) &&
	    (sm1 == IMM) && (i1->src.disp == 4) && (dm1 == REG) && ISA(dr1))
		{
		if(i1->label) goto end32;
		for(ti2=i1->prev;ti2;ti2=ti2->prev)
			{
			if(ti2->src.amode == REGI && ti2->src.areg == dr1)
				{
				if(ti2->flags!=LENL) goto end32;
				ti2->src.amode |= INC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+40000)
			    	goto true2;
				}
			if(ti2->dst.amode == REGI && ti2->dst.areg == dr1)
				{
				if(ti2->flags!=LENL) goto end32;
				ti2->dst.amode |= INC;
			    	delinst(i1);
			    	DBG(p2_line=__LINE__+40000)
			    	goto true2;
				}
			if(uses(ti2, dr1)) goto end32;
			if(breakflow(ti2)) goto end32;
			}
		}
end32:	ifn(i2 = i1->next) goto next;   /* put here because previous opts. */
	if(i2->label) goto next;	/* don't rely on i2 */
	op2 = i2->opcode;

	/*
	 * avoid too many reg moves..
	 */

	/*
	 *	move[q].x X,Ax		=>	move[q].x X,Ay
	 *	....				....
	 *	<stuff1>			<stuff2>
	 *	....				....
	 *	move.x	  Ax,Ay		=>	<deleted>
	 *
	 * stuff2 is stuff1 where Ax is replaced by Ay...
	 * Where Ax is dead and stuff does not ref nor set Ay and
	 * uses Ax with .x and contains no branch to subroutine.
	 */
	if(OP_MOVE(op1) && dm1 == REG && ISA(dr1) && !(i1->refs & RM(dr1)))
		{int Ay;
		register INST *ti1;
		ti1 = ti2 = i2;
		while(!(OP_MOVE(ti2->opcode) &&
		        ti2->src.amode == REG && ti2->src.areg == dr1 &&
		        ti2->dst.amode == REG && ISA(ti2->dst.areg) ))
			{
			if(ti2->next == NULL) goto end33;
			if(ti2->opcode == JSR || ti2->opcode == BSR) goto end33;
			ti2 = ti2->next;
			if(breakflow(ti2)) goto end33;
			}
		if(ti2->live & RM(dr1)) goto end33;
/*		if(((i1->flags&7)?(i1->flags&7):LENL) >
		   ((ti2->flags&7)?(ti2->flags&7):LENL)) goto end33;	/**/
		while(ti1 != ti2)
			{
			if(uses(ti1, ti2->dst.areg)) goto end33;
			if((ti1->refs&RM(dr1)) && !(ti1->sets&RM(dr1))) 
				goto end33;
			if(uses(ti1, dr1) && 
			   (ti2->flags&7)<(ti1->flags&7)) goto end33;	/**/
			ti1=ti1->next;
			}
		/*.
		 *	replace Ax by Ay
		 */
		ti1 = i1;Ay=ti2->dst.areg;
		while(ti1 != ti2)
			{int m;
			ti1->live = ti1->refs = ti1->sets = -1;
			m=M(ti1->src.amode);
			if(m==REG && ti1->src.areg==dr1) ti1->src.areg=Ay;
			if(m==REGI && ti1->src.areg==dr1) ti1->src.areg=Ay;
			if(m==REGID && ti1->src.areg==dr1) ti1->src.areg=Ay;
			if(m==REGIDX && ti1->src.areg==dr1) ti1->src.areg=Ay;
			if(m==REGIDX && ti1->src.ireg==dr1) ti1->src.ireg=Ay;
			m=M(ti1->dst.amode);
			if(m==REG && ti1->dst.areg==dr1) ti1->dst.areg=Ay;
			if(m==REGI && ti1->dst.areg==dr1) ti1->dst.areg=Ay;
			if(m==REGID && ti1->dst.areg==dr1) ti1->dst.areg=Ay;
			if(m==REGIDX && ti1->dst.areg==dr1) ti1->dst.areg=Ay;
			if(m==REGIDX && ti1->dst.ireg==dr1) ti1->dst.ireg=Ay;
			ti1=ti1->next;
			}
	    	delinst(ti2);
	    	DBG(p2_line=__LINE__+40000)
	    	goto true;
		}
end33:	/*
	 *	move[q].x X,Dx		=>	move[q].x X,Dy
	 *	....				....
	 *	<stuff1>			<stuff2>
	 *	....				....
	 *	move.x    Dx,Dy		=>	<deleted>
	 *
	 * stuff2 is stuff1 where Dx is replaced by Dy...
	 * Where Dx is dead and stuff does not ref nor set Dy and
	 * uses Dx with .x (or < .x) and contains no branch to 
	 * subroutine.
	 * UNSAFE since the last move may set the flags and is deleted.
	 */
	if(!safe && OP_MOVE(op1) && dm1 == REG && ISD(dr1) && !(i1->refs & RM(dr1)))
		{
		register INST *ti1;int Dy;
		ti1 = ti2 = i2;
		/*.
		 *	find 2nd move.x
		 */
		while(!(OP_MOVE(ti2->opcode) &&
		        ti2->src.amode == REG && ti2->src.areg == dr1 &&
		        ti2->dst.amode == REG && ISD(ti2->dst.areg) ))
			{
			if (ti2->next == NULL) goto end34;
			if (ti2->opcode == JSR || ti2->opcode == BSR) goto end34;
			ti2 = ti2->next;
			if (breakflow(ti2))    goto end34;
			}
		if(ti2->live & RM(dr1))		         goto end34;
		if(safe &&
		   ((i1->flags&7)?(i1->flags&7):LENL) >
		   ((ti2->flags&7)?(ti2->flags&7):LENL)) goto end34;	/**/
		if(safe && !cc_modified(ti2->next))      goto end34;
		/*.
		 *	check if stuff refs or sets Dy 
		 */
		while(ti1 != ti2)
			{
			if(uses(ti1, ti2->dst.areg)) goto end34;
			if((ti1->refs&RM(dr1)) && !(ti1->sets&RM(dr1))) 
				goto end34;
			if(uses(ti1, dr1) && 
			   (ti2->flags&7)<(ti1->flags&7)) goto end34;	/**/
			ti1=ti1->next;
			}
		/*.
		 *	replace Dx by Dy
		 */
		ti1 = i1;Dy=ti2->dst.areg;
		while(ti1 != ti2)
			{int m;
			ti1->live = ti1->refs = ti1->sets = -1;
			m=M(ti1->src.amode);
			if(m==REG && ti1->src.areg==dr1) ti1->src.areg=Dy;
			if(m==PCDX && ti1->src.ireg==dr1) ti1->src.ireg=Dy;
			if(m==REGIDX && ti1->src.ireg==dr1) ti1->src.ireg=Dy;
			m=M(ti1->dst.amode);
			if(m==REG && ti1->dst.areg==dr1) ti1->dst.areg=Dy;
			if(m==PCDX && ti1->dst.ireg==dr1) ti1->dst.ireg=Dy;
			if(m==REGIDX && ti1->dst.ireg==dr1) ti1->dst.ireg=Dy;
			ti1=ti1->next;
			}
	    	delinst(ti2);
	    	DBG(p2_line=__LINE__+40000)
	    	goto true;
		}
end34:	/*
	 *	sub.w	#1,Dx		=>	db<cc>	Dx,lbl
	 *	b<cc>	lbl			b<cc>	lbl
	 */
	if(!size && OP_SUB(op1) && sm1==IMM && i1->src.disp==1 && 
	   dm1==REG && ISD(dr1) && db_cc(op2) && (i1->flags & LENW))
	   	{
	   	setupinst(i1,db_cc(op2),0,REG,dr1,(ABS|SYMB),
						strsave(i2->src.astr));
	    	DBG(p2_line=__LINE__+40000)
	    	goto true;
	   	}
	/*
	 *	move.x	#n,Dx		=>	move.x	#n-1,Dx	
	 *	...(1)				...(1)
	 *	bra	lbl1			bra	lbl2
	 * 	...(2)				...(2)
	 * lbl1	dbf	Dx,lbl2		  lbl1	dbf	Dx,lbl2
	 *	...(3)			  lbl3	...(3)
	 *
	 * Where Dx is not used in (1). If n==0 use (bra lbl3) else 
	 * use (bra lbl2).
	 */
	if(OP_MOVE(op1) && sm1==IMM && dm1==REG && ISD(dr1))
		{register INST *bra,*dbf;

		ifn(bra=i2) goto end35;
		while(bra && !breakflow(bra) &&!uses(bra,dr1)) bra=bra->next;
		ifn(bra) goto end35;
		if(uses(bra,dr1)) goto end35;
		if(bra->opcode!=BRA) goto end35;

		ifn(dbf=find_label(bra->src.astr)) goto end35;
		if(dbf->opcode!=DBF) goto end35;
		if(dbf->src.areg!=dr1) goto end35;
		ifn(i1->src.disp&65535) 
			{char *lbl3;INST *ni;
			ifn (ni=dbf->next) goto end35;
			i1->opcode = MOVE;
			if(i1->flags==LENW) i1->src.disp=-1;
			else i1->src.disp |= 65535;
			if(ni->opcode==CLR && ni->flags==LENW && 
			   ni->src.amode==REG && ni->src.areg==dr1)
			   	{
			   	i1->src.disp &=~65535;
			   	if(ni->next) ni=ni->next;
			   	}
			if(!i1->src.disp && OP_SUB(ni->opcode) && 
			   ni->src.amode==IMM && ni->src.disp==1 && 
			   ni->dst.amode==REG && ni->dst.areg==dr1 && 
			   ni->flags==LENL && ni->next)
			   	{
			   	if(ni->next->opcode==BCC && ni->next->next) 
					{
					i1->src.disp=-1;
					i1->opcode=MOVEQ;
					i1->flags=0;
					ni=ni->next->next;
					}
			   	}
			ifn (lbl3=ni->label) 
			     lbl3=ni->label=strsave(mktmp());
			free(bra->src.astr);bra->src.astr = strsave(lbl3);
		    	DBG(p2_line=__LINE__+40000)
		    	goto true;
			}
		i1->src.disp = (i1->src.disp&~65535)|((i1->src.disp-1) & 65535);
		i1->opcode = MOVE;
		if(i1->flags==LENL && D8OK(i1->src.disp))	
			{
			i1->opcode=MOVEQ;
			i1->flags=0;
			} 
		if(bra->next && bra->next->label &&
		   !strcmp(bra->next->label,dbf->dst.astr))
		   	{
		   	if(!islabelused(bra->next->label))
		   		{
		   		free(bra->next->label);
		   		bra->next->label = NULL;
		   		}
		   	delinst(bra);
		   	++s_bdel;
		   	}
		else	{
			free(bra->src.astr);
			bra->src.astr=strsave(dbf->dst.astr);
			}
	    	DBG(p2_line=__LINE__+40000)
	    	goto true;
	    	}
end35:
next:	return ipeep2_5(i1);
true:	return i1->next;
true2:	return i2;
	}
