/*.
 *	movem.c - remove unneeded regs in movem couples.
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

/*.
 *	proc_label() - is ip a beginning of a function (label begins
 *		       with '_' or '@')
 */
static bool proc_label(ip)
	INST *ip;
	{
	char *s;
	ifn(s = ip->label) return FALSE;
	return *s=='_' || *s=='@';
	}

/*.
 *	optim() - optimize & return next label
 */
/*
 *	_LABEL1	...		=>	_LABEL1	...
 *		movem.x	MSK,-(sp)		movem.x	MSK2,-(sp)
 *		...				sub	#N,sp
 *		...				...
 *		...				...
 *		...				add	#N,sp
 *		movem.x	(sp)+,MSK		movem.x	(sp)+,MSK2
 *		...				...
 *	_LABEL2	...			_LABEL2	...
 *
 * MSK2 is MSK without unused regs between the two labels.
 * Delete or replace movem by move if needed. N is used to
 * fix stack offset. On a 68000 this doesn't improve speed if
 * x=.w and difference between MSK and MSK2 is less than two bits.
 *
 * Note: substituting M(SP) by M-N(SP) between the labels is 
 * too unsafe.
 */
static INST *optim(ibeg)
	INST *ibeg;
	{
	int mask,umask,nb_unused;
	INST *iend;
	INST *movem1=NULL,*movem2=NULL,*ip;
	bool modified;
	bool hack1 = FALSE,hack2 = FALSE;

	/*.
	 *	find end
	 */
	for(iend=ibeg->next;iend && !proc_label(iend);iend=iend->next);
	
	/*.
	 *	find 1st & 2nd movem 
	 */
	for(ip=ibeg;ip!=iend;ip=ip->next)
		{
		/* hack to treat 
		 *	MOVE.L Ax,-(sp) as a MOVEM 
                 */
		if(!movem1 && !hack1 && 
		   ip->opcode==MOVE && ip->src.amode==REG &&
		   ip->dst.amode==(REGI|DEC) && ip->dst.areg==SP &&
		   ip->flags==LENL)
		   	{
		   	ip->opcode    = MOVEM;
		   	ip->src.amode = MSK;
		   	ip->src.disp  = RM(ip->src.areg);
		   	hack1 = TRUE;
		   	}
		/* hack to treat 
		 *	PEA (Ax) as a MOVEM 
                 */
		if(!movem1 && !hack1 && 
		   ip->opcode==PEA && ip->src.amode==REGI)
		   	{
		   	ip->opcode    = MOVEM;
		   	ip->src.amode = MSK;
		   	ip->src.disp  = RM(ip->src.areg);
		   	ip->dst.areg  = SP;
		   	ip->dst.amode = REGI|DEC;
		   	ip->flags     = LENL;
		   	hack1 = TRUE;
		   	}
		if(ip->opcode==MOVEM && ip->src.amode==MSK &&
		   ip->dst.amode==(REGI|DEC) && ip->dst.areg==SP)
		   	{
		   	ifn(movem1) movem1=ip;
		   	else	{
		   		fprintf(stderr,
				"%s: Warning : movem used twice !!\n",
				ProgName);
				dumpline(movem1);
				goto end;	/* 1st movem twice !! */
				}
		   	}
		/* hack to treat 
		 *	MOVE.L (sp)+,Ax as a MOVEM 
                 */
		if(!movem2 && !hack2 && 
		   ip->opcode==MOVE && ip->dst.amode==REG &&
		   ip->src.amode==(REGI|INC) && ip->src.areg==SP &&
		   ip->flags==LENL)
		   	{
		   	ip->opcode    = MOVEM;
		   	ip->dst.amode = MSK;
		   	ip->dst.disp  = RM(ip->dst.areg);
		   	hack2 = TRUE;
		   	}
		if(ip->opcode==MOVEM && ip->dst.amode==MSK &&
		   ip->src.amode==(REGI|INC) && ip->src.areg==SP)
		   	{
		   	ifn(movem2) movem2=ip;
		   	else	{
		   		fprintf(stderr,
				"%s: Warning : movem used twice !!\n",
				ProgName);
				dumpline(movem2);
				goto end;	/* 2nd movem twice !! */
				}
		   	}
		}
	/*.
	 *	check matching
	 */
	ifn(movem1) goto end;
	ifn(movem2) goto end;
	if(movem1->src.disp!=movem2->dst.disp)	/* movem mask mismatch !! */
		{
		fprintf(stderr,"%s: Warning : movem mismatch !!\n",ProgName);
		dumpline(movem1);
		dumpline(movem2);
		goto end;
		}
	/*.
	 *	get unused regs
	 */
	umask = 0;
	for(ip=ibeg;ip!=iend;ip=ip->next) 
		if(ip!=movem1 && ip!=movem2 && 
		!safe && ip->opcode!=RTS && ip->opcode!=BSR && 
			 ip->opcode!=JSR) umask |= (ip->refs|ip->sets);

	/*.
	 *	optimize
	 */
	if(umask & RM(SP))
		{
		nb_unused = nb_bits((movem1->src.disp & ~umask)&65535);
		ifn(nb_unused) goto end; /* all pushed regs are used */

		if(nb_bits(movem1->src.disp & umask)>1 && 
		   nb_unused<=2 && movem1->flags==LENW && !mc40) goto end;

		nb_unused <<= (movem1->flags&LENW)?1:
			      (movem1->flags&LENB)?0:2;

		if(size && nb_bits(movem1->src.disp & umask)>1) goto end;

		if(nb_unused<=8)
			{
			setupinst(insinst(movem1),SUBQ,LENL,IMM,nb_unused,
								REG,SP);
			setupinst(inspreinst(movem2),ADDQ,LENL,IMM,nb_unused,
								REG,SP);
			}
		else	{
			setupinst(insinst(movem1),LEA,0,REGID,-nb_unused,SP,
								REG,SP);
			setupinst(inspreinst(movem2),LEA,0,REGID,nb_unused,SP,
								REG,SP);
			}
		}
	ifn(mask = movem2->dst.disp = (movem1->src.disp &= umask))
		{ 
		if(movem1==iend) iend = iend->next;
		delinst(movem1);
		if(movem2==iend) iend = iend->next;
		delinst(movem2);
		hack2=hack1=0; 
		modified = TRUE;
		}
	else if(nb_bits(mask)==1)
		{
	    	movem1->opcode    = MOVE;
	    	movem1->src.amode = REG;
		movem1->src.areg  = first_nb(mask);
	    	movem2->opcode    = MOVE;
	    	movem2->dst.amode = REG;
		movem2->dst.areg  = first_nb(mask);
		modified = TRUE;
		}
	if(modified)
		{
		++s_movem;
		DBG(printf("(%s) ",ibeg->label);fflush(stdout))
		}
end:	return iend;
	}

/*
 * movem40src(): movem MSK,-(Ax) => multiples moves. 
 * return next inst. (Opt. suggested by P.Lauly)
 */
static INST *movem40src(ip)
	INST *ip;
	{
	int msk=ip->src.disp;
	int i;
	INST *ni;
	if(ip->dst.amode!=(REGI|DEC)) return ip->next;
	if(size && (nb_bits(msk)>2)) return ip->next;
	ni = ip;
	for(i=A7;i>=A0;--i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						REG,i,
						(REGI|DEC),ip->dst.areg);
		}
	for(i=D7;i>=D0;--i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						REG,i,
						(REGI|DEC),ip->dst.areg);
		}
	ni=ip->next;delinst(ip);
	++s_movem;++s_mc40;
	return ni;
	}

/*
 * movem40dst() : movem (Ax)+,MSK => multiples moves. 
 * return next inst. (Opt. suggested by P.Lauly)
 */
static INST *movem40dst(ip)
	INST *ip;
	{
	int msk=ip->dst.disp;
	int i;
	INST *ni;
	if(ip->src.amode!=(REGI|INC)) return ip->next;
	if(size && (nb_bits(msk)>2)) return ip->next;
	ni = ip;
	for(i=D0;i<=D7;++i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						(REGI|INC),ip->src.areg,
						REG,i);
		}
	for(i=A0;i<=A7;++i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						(REGI|INC),ip->src.areg,
						REG,i);
		}
	ni=ip->next;delinst(ip);
	++s_movem;++s_mc40;
	return ni;
	}

/*
 * movem20src() : movem MSK,-(Ax) => multiples moves 
 * (if <= 2 regs). return next inst. (Opt. suggested 
 * by L.Marechal)
 */
static INST *movem20src(ip)
	INST *ip;
	{
	int msk=ip->src.disp;
	int i;
	INST *ni;
	if(ip->dst.amode!=(REGI|DEC)) return ip->next;
	if(size && (nb_bits(msk)>2)) return ip->next;
	if(nb_bits(msk)>2) return ip->next;
	ni = ip;
	for(i=A7;i>=A0;--i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						REG,i,
						(REGI|DEC),ip->dst.areg);
		}
	for(i=D7;i>=D0;--i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						REG,i,
						(REGI|DEC),ip->dst.areg);
		}
	ni=ip->next;delinst(ip);
	++s_movem;++s_mc20;
	return ni;
	}

/*
 * movem20dst() : movem (Ax)+,MSK => multiples moves 
 * (if <= 10 regs). return next inst. (Opt. suggested 
 * by L.Marechal)
 */
static INST *movem20dst(ip)
	INST *ip;
	{
	int msk=ip->dst.disp;
	int i;
	INST *ni;
	if(ip->src.amode!=(REGI|INC)) return ip->next;
	if(size && (nb_bits(msk)>2)) return ip->next;
	if(nb_bits(msk)>10) return ip->next;
	ni = ip;
	for(i=D0;i<=D7;++i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						(REGI|INC),ip->src.areg,
						REG,i);
		}
	for(i=A0;i<=A7;++i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						(REGI|INC),ip->src.areg,
						REG,i);
		}
	ni=ip->next;delinst(ip);
	++s_movem;++s_mc20;
	return ni;
	}

/*
 * movem00src() : movem MSK,-(Ax) => multiples moves 
 * (if <= 2 regs).
 */
static INST *movem00src(ip)
	INST *ip;
	{
	int msk=ip->src.disp;
	int i;
	INST *ni;
	if(ip->dst.amode!=(REGI|DEC)) return ip->next;
	if(size && (nb_bits(msk)>2)) return ip->next;
	if(nb_bits(msk)>2) return ip->next;
	ni = ip;
	for(i=A7;i>=A0;--i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						REG,i,
						(REGI|DEC),ip->dst.areg);
		}
	for(i=D7;i>=D0;--i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						REG,i,
						(REGI|DEC),ip->dst.areg);
		}
	ni=ip->next;delinst(ip);
	++s_movem;
	return ni;
	}

/*
 * movem00dst() : movem (Ax)+,MSK => multiples moves 
 * (if <= 3 regs).
 */
static INST *movem00dst(ip)
	INST *ip;
	{
	int msk=ip->dst.disp;
	int i;
	INST *ni;
	if(ip->src.amode!=(REGI|INC)) return ip->next;
	if(size && (nb_bits(msk)>2)) return ip->next;
	if(nb_bits(msk)>3) return ip->next;
	ni = ip;
	for(i=D0;i<=D7;++i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						(REGI|INC),ip->src.areg,
						REG,i);
		}
	for(i=A0;i<=A7;++i) if(msk & RM(i))
		{
		ni = setupinst(insinst(ni),MOVE,ip->flags,
						(REGI|INC),ip->src.areg,
						REG,i);
		}
	ni=ip->next;delinst(ip);
	++s_movem;
	return ni;
	}

/*.
 * movem() - process optimisation
 */
void movem()
	{
	register INST	*ip;

	DBG(printf("movem    : "));
	ip = head;
	while(ip)
		{
		while(ip && !proc_label(ip)) ip=ip->next;
		if(ip) ip = optim(ip);
		}
	if(mc40)
		{
		DBG(printf("68040 "));
		for(ip=head;ip;) 
			{
			if(ip->opcode==MOVEM) 
				{
				if(ip->src.amode==MSK) ip = movem40src(ip);
				else ip = movem40dst(ip);
				}
			else	ip=ip->next;
			}
		}
	else if(mc20)
		{
		DBG(printf("68020 "));
		for(ip=head;ip;) 
			{
			if(ip->opcode==MOVEM) 
				{
				if(ip->src.amode==MSK) ip = movem20src(ip);
				else ip = movem20dst(ip);
				}
			else	ip=ip->next;
			}
		}
	else
		{
		DBG(printf("68000 "));
		for(ip=head;ip;) 
			{
			if(ip->opcode==MOVEM) 
				{
				if(ip->src.amode==MSK) ip = movem00src(ip);
				else ip = movem00dst(ip);
				}
			else	ip=ip->next;
			}
		}
	DBG(printf(" \n"));
	}
