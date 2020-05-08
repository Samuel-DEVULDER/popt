/*.
 *	braopt.c - branch optimization (branch to branch & branch to next,
 *	branch inversion, dbf optimizations).
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"
#include <strings.h>

#define	LAB_TOUCHED	0x80
#define	OP_BRANCH(x)	(((x)->opcode==BRA || (x)->opcode==JMP) && \
			 (((x)->src.amode&~XWORD)==(ABS|SYMB) || \
			  (x)->src.amode==(PCD|SYMB)))

/*.
 * cnb() - optimize bcc followed by bra. return next inst.
 *
/*
 *		...				..__	
 *		b<cc>	lbl2	=>		b<cc>	lbl1
 *		bra	lbl1			<deleted>
 *	lbl2	...			lbl2	...
 */
static INST *cnb(bcc)
	INST *bcc;
	{
	INST *bra;
	char *lbl2,*lbl3;
	
	bra = bcc->next;
	ifn(bra->next) goto fail;
	ifn(lbl2 = bra->next->label) goto fail;
	if(strcmp(bcc->src.astr,lbl2)) goto fail;

	DBG(printf("cnb(%d) ",bcc->lineno);fflush(stdout));

	bcc->opcode = bcomp(bcc->opcode);++s_brev;
	free(bcc->src.astr);bcc->src.astr = strsave(bra->src.astr);
	delinst(bra);++s_bdel;
fail:	return bcc->next;	
	}

/*.
 * b2c() - optimize bra to bcc. return next inst.
 */
/*
 *		...				..__	
 *		bra	lbl1	=>		b<cc>	lbl3
 *	lbl2	...			lbl2	...
 *		...				...
 *	lbl1	b<cc>	lbl2		lbl1	b<cc>	lbl2
 *		...			lbl3	...
 */
static INST *b2c(bra,bcc)
	INST *bra,*bcc;
	{
	char *lbl2,*lbl3;
	
	ifn(bra->next) goto fail;
	ifn(lbl2 = bra->next->label) goto fail;
	if(strcmp(bcc->src.astr,lbl2)) goto fail;
	ifn(bcc->next) goto fail;

	DBG(printf("b2c(%d) ",bra->lineno);fflush(stdout));

	bra->opcode = bcomp(bcc->opcode);++s_brev;
	ifn(lbl3 = bcc->next->label) bcc->next->label=lbl3=strsave(mktmp());
	free(bra->src.astr);
	bra->src.astr = strsave(lbl3);
fail:	return bra->next;	
	}

/*.
 * mbc() - optimize a Move followed by a Branch to a Cmp. return next inst.
 */
/*
 *
 *		move.y	#n,Rn	=>		move.y	#n,Rn
 *		...(1)				...(1)
 *		bra	lbl1			bra	XXX
 *		...(2)				...(2)
 *	lbl1	cmp.x	#m,Rn		  lbl1	cmp.x	#m,Rn
 *		b<cc>	lbl2			b<cc>	lbl2
 *		...(3)			  lbl3	...(3)
 *
 * Where Rn is not set in (1), y>=x. XXX is lbl3 if b<cc> is true
 * or lbl2 else.
 */
static INST *mbc(bra,cmp)
	INST *bra,*cmp;
	{
	INST *mov,*bcc;
	int Rn;
	
	if(cmp->src.amode!=IMM || cmp->dst.amode!=REG) goto fail;
	Rn=cmp->dst.areg;
	if(!(bcc=cmp->next) || !bcomp(bcc->opcode)) goto fail;
	/*.
	 *	Find move
	 */
	for(mov=bra->prev;mov;mov=mov->prev)
		{
		if(OP_MOVE(mov->opcode) && mov->dst.amode==REG &&
			mov->dst.areg==Rn) break;
		if(mov->sets & RM(Rn)) goto fail;
		if(breakflow(mov)) goto fail;
		}
	ifn(mov) goto fail;
	if(mov->src.amode!=IMM) goto fail;
	if((mov->flags?mov->flags:4)<cmp->flags) goto fail;
	
	Rn=evaltst(bcc->opcode,cmp->flags,mov->src.disp-cmp->src.disp);
	if(Rn==3) goto fail;

	DBG(printf("mbc(%d) ",bra->lineno);fflush(stdout));

	if(Rn)	{
		if(bra->next->label && 
		   !strcmp(bcc->src.astr,bra->next->label))
		   	{INST *ni=bra->next;
		   	delinst(bra);
		   	bra = ni;
		   	}
		else	{
			free(bra->src.astr);
			bra->src.astr = strsave(bcc->src.astr);
			}
		}
	else	{ char *lbl3;
		ifn(lbl3 = bcc->next->label) 
			lbl3=bcc->next->label=strsave(mktmp());
		free(bra->src.astr);
		bra->src.astr = strsave(lbl3);
		}

fail:	return bra->next;	
	}

/*.
 * mbmc2() - optimize a Move followed by a Branch to a move and a Cmp. 
 *	return next inst.
 */
/*
 *		move.y	#n,Rn	=>		move.y	#n,Rn
 *		...(1)				...(1)
 *		bra	lbl1			bra	XXX
 *		...(2)				...(2)
 *	lbl1	move	#m,Ry		  lbl1	move	#m,Ry
 *		cmp.x	Rn,Ry		  	cmp.x	Rn,Ry
 *		b<cc>	lbl2			b<cc>	lbl2
 *		...(3)			  lbl3	...(3)
 *
 * Where Rn is not set in (1), y>=x. XXX is lbl3 if b<cc> is true
 * or lbl2 else. And where Ry is dead after the cmp.
 */
static INST *mbmc2(bra,mov1)
	INST *bra,*mov1;
	{
	INST *mov,*bcc,*cmp;
	int Rn;
	
	if(mov1->src.amode!=IMM || mov1->dst.amode!=REG) goto fail;
	
	cmp=mov1->next;
	
	if(!OP_CMP(cmp->opcode) || cmp->src.amode!=REG || 
		cmp->dst.amode!=REG) goto fail;

	if(cmp->live & RM(mov1->dst.areg)) goto fail;
	if(mov1->dst.areg!=cmp->dst.areg) goto fail;

	Rn=cmp->src.areg;

	if(!(bcc=cmp->next) || !bcomp(bcc->opcode)) goto fail;
	/*.
	 *	Find move
	 */
	for(mov=bra->prev;mov;mov=mov->prev)
		{
		if(OP_MOVE(mov->opcode) && mov->dst.amode==REG &&
			mov->dst.areg==Rn) break;
		if(mov->sets & RM(Rn)) goto fail;
		if(breakflow(mov)) goto fail;
		}
	ifn(mov) goto fail;
	if(mov->src.amode!=IMM) goto fail;
	if((mov->flags?mov->flags:4)<cmp->flags) goto fail;
	
	Rn=evaltst(bcc->opcode,cmp->flags,mov1->src.disp-mov->src.disp);
	if(Rn==3) goto fail;

	DBG(printf("mbmc2(%d) ",bra->lineno);fflush(stdout));

	if(Rn)	{
		if(bra->next->label && 
		   !strcmp(bcc->src.astr,bra->next->label))
		   	{INST *ni=bra->next;
		   	delinst(bra);
		   	bra = ni;
		   	}
		else	{
			free(bra->src.astr);
			bra->src.astr = strsave(bcc->src.astr);
			}
		}
	else	{ char *lbl3;
		ifn(lbl3 = bcc->next->label) 
			lbl3=bcc->next->label=strsave(mktmp());
		free(bra->src.astr);
		bra->src.astr = strsave(lbl3);
		}

fail:	return bra->next;	
	}

/*.
 * mbmc1() - optimize a Move followed by a Branch to a move and a Cmp. 
 *	return next inst.
 */
/*
 *		move.y	#n,Rn	=>		move.y	#n,Rn
 *		...(1)				...(1)
 *		bra	lbl1			bra	XXX
 *		...(2)				...(2)
 *	lbl1	move	#m,Dy		  lbl1	move	#m,Dy
 *		cmp.x	Dy,Rn		  	cmp.x	Dy,Rn
 *		b<cc>	lbl2			b<cc>	lbl2
 *		...(3)			  lbl3	...(3)
 *
 * Where Rn is not set in (1), y>=x. XXX is lbl3 if b<cc> is true
 * or lbl2 else. And Dy is dead after the cmp.
 */
static INST *mbmc1(bra,mov1)
	INST *bra,*mov1;
	{
	INST *mov,*bcc,*cmp;
	int Rn;
	
	if(mov1->src.amode!=IMM || mov1->dst.amode!=REG) goto fail;
	
	cmp=mov1->next;
	
	if(!OP_CMP(cmp->opcode) || cmp->src.amode!=REG || 
		cmp->dst.amode!=REG) goto fail;

	if(cmp->live & RM(mov1->dst.areg)) goto fail;
	if(mov1->dst.areg!=cmp->src.areg) goto fail;

	Rn=cmp->dst.areg;

	if(!(bcc=cmp->next) || !bcomp(bcc->opcode)) goto fail;
	/*.
	 *	Find move
	 */
	for(mov=bra->prev;mov;mov=mov->prev)
		{
		if(OP_MOVE(mov->opcode) && mov->dst.amode==REG &&
			mov->dst.areg==Rn) break;
		if(mov->sets & RM(Rn)) goto fail;
		if(breakflow(mov)) goto fail;
		}
	ifn(mov) goto fail;
	if(mov->src.amode!=IMM) goto fail;
	if((mov->flags?mov->flags:4)<cmp->flags) goto fail;
	
	Rn=evaltst(bcc->opcode,cmp->flags,mov->src.disp-mov1->src.disp);
	if(Rn==3) goto fail;

	DBG(printf("mbmc1(%d) ",bra->lineno);fflush(stdout));

	if(Rn)	{
		if(bra->next->label && 
		   !strcmp(bcc->src.astr,bra->next->label))
		   	{INST *ni=bra->next;
		   	delinst(bra);
		   	bra = ni;
		   	}
		else	{
			free(bra->src.astr);
			bra->src.astr = strsave(bcc->src.astr);
			}
		}
	else	{ char *lbl3;
		ifn(lbl3 = bcc->next->label) 
			lbl3=bcc->next->label=strsave(mktmp());
		free(bra->src.astr);
		bra->src.astr = strsave(lbl3);
		}

fail:	return mbmc2(bra,mov1);	
	}

/*.
 * mbt() - optimize a Move followed by a Branch to a Tst. return next inst.
 */
/*
 *		move.y	#n,Rn	=>		move.y	#n,Rn
 *		...(1)				...(1)
 *		bra	lbl1			bra	XXX
 *		...(2)				...(2)
 *	lbl1	tst.x	Rn		  lbl1	tst.x	Rn
 *		b<cc>	lbl2			b<cc>	lbl2
 *		...(3)			  lbl3	...(3)
 *
 * Where Rn is not set in (1), y>=x. XXX is lbl3 if b<cc> is true 
 * or lbl2 else.
 */
static INST *mbt(bra,tst)
	INST *bra,*tst;
	{
	INST *mov,*bcc;
	int Rn;
	
	if(tst->src.amode!=REG) goto fail;
	Rn=tst->src.areg;
	if(!(bcc=tst->next) || !bcomp(bcc->opcode)) goto fail;
	/*.
	 *	Find move
	 */
	for(mov=bra->prev;mov;mov=mov->prev)
		{
		if(OP_MOVE(mov->opcode) && mov->dst.amode==REG &&
			mov->dst.areg==Rn) break;
		if(mov->sets & RM(Rn)) goto fail;
		if(breakflow(mov)) goto fail;
		}
	ifn(mov) goto fail;
	if(mov->src.amode!=IMM) goto fail;
	if((mov->flags?mov->flags:4)<tst->flags) goto fail;
	
	Rn=evaltst(bcc->opcode,tst->flags,mov->src.disp);
	if(Rn==3) goto fail;

	DBG(printf("mbt(%d) ",bra->lineno);fflush(stdout));

	if(Rn)	{
		if(bra->next->label && 
		   !strcmp(bcc->src.astr,bra->next->label))
		   	{INST *ni=bra->next;
		   	delinst(bra);
		   	bra = ni;
		   	}
		else	{
			free(bra->src.astr);
			bra->src.astr = strsave(bcc->src.astr);
			}
		}
	else	{ char *lbl3;
		ifn(lbl3 = bcc->next->label) 
			lbl3=bcc->next->label=strsave(mktmp());
		free(bra->src.astr);
		bra->src.astr = strsave(lbl3);
		}

fail:	return bra->next;	
	}

/*.
 *	braopt(ip) - perform optimisation for bra inst.
 *	return next inst.
 */
static INST *braopt(ip)
	INST *ip;
	{
	INST *ni,*ret,*ip2;
	int doublejump = FALSE;

	ifn(ni = find_label(ip->src.astr))
		{
err:		if(chkxref(ip->src.astr))
			{
			return ip->next;
			}
		else	{
			fprintf(stderr,
				"%s: braopt() - label not found: %s !\n",
				ProgName,ip->src.astr);
			dumpline(ip);
			exit(1);
			}
		}
	ip2 = ip->next;
	if(bcomp(ni->opcode))
		{
		ret = b2c(ip,ni);
		if(ret != ip2)   return ret;
		}
	if(ni->opcode==TST)
		{
		ret = mbt(ip,ni);
		if(ret != ip2)   return ret;
		}
	if(OP_CMP(ni->opcode))
		{
		ret = mbc(ip,ni);
		if(ret != ip2)   return ret;
		}
	if(OP_MOVE(ni->opcode))
		{
		ret = mbmc1(ip,ni);
		if(ret != ip2)   return ret;
		}

	while(OP_BRANCH(ni) && !(ni->flags&LAB_TOUCHED))
		{
		ni->flags |= LAB_TOUCHED;doublejump = TRUE;
		ifn(ni = find_label(ni->src.astr)) goto err;
		if(ni->flags & LAB_TOUCHED)
			{
			fprintf(stderr,"%s: braopt() - Branch looping !\n",
				ProgName);
			clear_touched();
			dumpline(ni);
			exit(1);
			}
		}
	if(ip->next==ni)
		{
		++s_bdel;
		DBG(printf("b2n(%d) ",ip->lineno);fflush(stdout));
		delinst(ip); 
		ip = ni;
		}
	else if(doublejump) 
		{
		DBG(printf("b2b(%d) ",ip->lineno);fflush(stdout));
		free(ip->src.astr);
		ip->src.astr = strsave(ni->label);
		clear_touched();
		++s_bdel;
		ip=ip->next;
		}
	else	ip=ip->next;
	return ip;
	}

/*.
 *	bccopt(ip) - perform optimisation for bcc inst. 
 *	return next inst.
 */
static INST *bccopt(ip)
	INST *ip;
	{
	INST *ni;
	int doublejump = FALSE;

	ifn(ni = find_label(ip->src.astr))
		{
err:		if(chkxref(ip->src.astr))
			{
			return ip->next;
			}
		else	{
			fprintf(stderr,
				"%s: braopt() - label not found: %s !\n",
				ProgName,ip->src.astr);
			dumpline(ip);
			exit(1);
			}
		}
	while(OP_BRANCH(ni) && !(ni->flags&LAB_TOUCHED))
		{
		ni->flags |= LAB_TOUCHED;doublejump = TRUE;
		ifn(ni = find_label(ni->src.astr)) goto err;
		if(ni->flags & LAB_TOUCHED)
			{
			fprintf(stderr,"%s: bccopt() - Branch looping !\n",
				ProgName);
			clear_touched();
			dumpline(ni);
			exit(1);
			}
		}
	if(ip->next==ni)
		{
		++s_bdel;
		DBG(printf("c2n(%d) ",ip->lineno);fflush(stdout));
		delinst(ip); 
		ip = ni;
		}
	else if(doublejump) 
		{
		DBG(printf("c2b(%d) ",ip->lineno);fflush(stdout));
		free(ip->src.astr);
		ip->src.astr = strsave(ni->label);
		clear_touched();
		++s_bdel;
		ip=ip->next;
		}
	else	ip=ip->next;
	return ip;
	}
/*.
 *	branchopt() - optimize bra statements. must be called after 
 *	labmerged().
 */
void branchopt()
	{
	INST *ip;

	DBG(printf("branchopt: ");fflush(stdout));
	for(ip=head;ip;) 
		{
		if(OP_BRANCH(ip)) ip = braopt(ip);
		else if(bcomp(ip->opcode))
			{
			if(ip->next && OP_BRANCH(ip->next)) 
				ip = cnb(ip);
			else	ip = bccopt(ip);
			}
		else	ip = ip->next;
		}
	ifn(keeplbl) cleanup();
	del_unreach();
	}
