/* 
 *	mulopt.c - optimization of multiplication. Some come from asp68k
 *		   project (mul* #7).
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

#define	C_ADD		((mc20 || mc40)?2:8)
#define	C_SH(m)		((mc20 || mc40)?4:(8 + (m<<1)))
#define	C_MOVE		((mc20 || mc40)?2:4)

/*
 * generate the best shift operation sequence
 */
#define	PUTSHIFT(ip,m,dx,ni,H,sz)					\
	if(m==0){}							\
	else if(m==1)							\
		{							\
		ni = setupinst(insinst(ni),ADD,LENL,REG,dx,REG,dx);	\
		H += C_ADD;	sz += 2;				\
		}							\
	else if(m<=8)							\
		{							\
		ni = setupinst(insinst(ni),ip->opcode==MULU?LSL:ASL,	\
						LENL,IMM,m,REG,dx);	\
		H += C_SH(m); sz += 2;					\
		}							\
	else	{							\
		int ds;							\
		for(ds=D0;ds<=D7 && ((ip->live & RM(ds)) || (ds==dx));	\
							++ds);		\
		if(ds<=D7)						\
			{						\
			ni = setupinst(insinst(ni),MOVEQ,LENL,IMM,m,	\
								REG,ds);\
			H += C_MOVE;	sz += 2;			\
			ni = setupinst(insinst(ni),ip->opcode==MULU?	\
					LSL:ASL,LENL,REG,ds,REG,dx);	\
			H += C_SH(m); sz += 2;				\
			}						\
		else	{						\
			while(m>8)					\
				{int mm;m -= 8;	mm = m<=8 ? m : 8;	\
				ni = setupinst(insinst(ni),		\
			ip->opcode==MULU?LSL:ASL,LENL,IMM,mm,REG,dx);	\
				H += C_SH(mm); sz += 2;			\
				}					\
			}						\
		}	

/*
 *	muloptw() - optimize word multiplication
 */
static bool muloptw(ip)
	INST *ip;
	{
	INST *ni;
	int   N,		/* nb of cycles of std mul inst      */
	      H,		/* --------------- shift&add version */
	      n,m,dx,ds,sz;
	bool  sign;

	if(ip->opcode!=MULU && ip->opcode!=MULS) goto false;
	ifn(ip->flags & LENW) goto false;
	if(ip->src.amode!=IMM) goto false;
	if(ip->dst.amode!=REG) goto false;

	n    = ip->src.disp;
	dx   = ip->dst.areg;
	ni   = ip;
	H    = 0;
	sz   = 0;
	sign = FALSE;

	/*
	 * Compute nb of cycles of mul inst (N)
	 */
	if(mc20) N = 28; 
	else if(ip->opcode==MULU) N = 44+(nb_bits(n&65535)<<1);
	else N = 44+nb_bits(n&65535/*(n ^ (n<<1))&((1<<17)-1)*/)<<1;
	
	/*
	 * Process translation & compute nb of cycles of the translated
	 * version (H)
	 */
	ifn(n)	{
		ni = setupinst(insinst(ni),MOVEQ,0,IMM,0,REG,dx);
		sz += 2;
		goto true;
		}
	
	if(ip->opcode==MULS)
		{
		ni = setupinst(insinst(ni),EXT,LENL,REG,dx,NONE); H += 4; 
		sz += 2;
		if(n<0) {sign = TRUE;n = -n;}
		}
	else	{
		if(mc20 || mc40)
			{
		ni = setupinst(insinst(ni),AND,LENL,IMM,65535,REG,dx);   
		H += 6; sz += 6;
			}
		else
			{
		ni = setupinst(insinst(ni),SWAP,0,REG,dx,NONE);   H += 4; sz += 2;
		ni = setupinst(insinst(ni),CLR,LENW,REG,dx,NONE); H += 4; sz += 2;
		ni = setupinst(insinst(ni),SWAP,0,REG,dx,NONE);   H += 4; sz += 2;
			}
		}

	n &= 65535;

	if(n==7)
		{
		for(ds=D0;ds<=D7 && ((ip->live & RM(ds)) || (ds==dx));++ds);
		if(ds<=D7)
			{
			ni = setupinst(insinst(ni),MOVE,LENL,REG,dx,REG,ds);
			ni = setupinst(insinst(ni),ip->opcode==MULU?LSL:ASL,
							LENL,IMM,3,REG,dx);
			ni = setupinst(insinst(ni),SUB,LENL,REG,ds,REG,dx);
			sz += 6;
			goto true;
			}
		}
		
	m = first_nb(n);
	if(nb_bits(n)==1 && 8<=m && m<=15)
		{
		ni = setupinst(insinst(ni),SWAP,0,REG,dx,NONE);
		ni = setupinst(insinst(ni),CLR,LENW,REG,dx,NONE); sz += 4;
		ni = setupinst(insinst(ni),ip->opcode==MULU?LSR:ASR,LENL,
							IMM,16-m,REG,dx); 
		sz += 2;
		goto true;
		}
	
	if(n!=1)
		{
		m = first_nb(n); n >>= m+1;
		PUTSHIFT(ip,m,dx,ni,H,sz)
		if(n!=0)
			{
			for(ds=D0;ds<=D7 && ((ip->live & RM(ds)) || (ds==dx));++ds);
			if(ds>D7) goto fail;
			ni->live |= RM(ds);
			ni = setupinst(insinst(ni),
				MOVE,LENL,REG,dx,REG,ds); H += C_MOVE; sz += 2;
			do	{
				m = first_nb(n)+1; n >>= m;
				PUTSHIFT(ip,m,ds,ni,H,sz)
				ni = setupinst(insinst(ni),
					ADD,LENL,REG,ds,REG,dx); H += C_ADD; sz += 2;
				}
			while(n);
			}
		}

	if(sign)
		{
		ni = setupinst(insinst(ni),NEG,LENL,REG,dx,NONE); sz += 2;
		H += (mc20 || mc40)?2:6;
		}
	
	/*
	 * keep the best version
	 */
	if(size && sz>4) goto fail;
	if(H>=N) 
		{
/*		DBG(printf("(%d>=%d) ",H,N));	/**/
		goto fail;
		}
	
true:	delinst(ip);		/* sucess */
/*	DBG(printf("mul "));	/**/
	return TRUE;

fail:	while(ni!=ip)		/* can't find scratch reg. */
		{INST *ti;
		ti = ni->prev;
		delinst(ni);--s_idel;
		ni = ti;
		}
		
false:	return FALSE;		/* can't optimize */
	}

/* UNAVAILABLE
 *	muloptl() - optimize long multiplication
 */
static bool muloptl(ip)
	INST *ip;
	{
	INST *ni;
	int   N,		/* nb of cycles of std mul inst      */
	      H,		/* --------------- shift&add version */
	      n,m,dx,ds,sz;
	bool  sign;

	if(ip->opcode!=MULU && ip->opcode!=MULS) goto false;
	ifn(ip->flags & LENL) goto false;
	if(ip->src.amode!=IMM) goto false;
	if(ip->dst.amode!=REG) goto false;

	n    = ip->src.disp;
	dx   = ip->dst.areg;
	ni   = ip;
	H    = 0;
	sz   = 0;
	sign = FALSE;

	/*
	 * Compute nb of cycles of mul inst (N)
	 */
	if(mc20) N = 44; 
	else if(ip->opcode==MULU) N = (44+(nb_bits(n&65535)<<1))<<1;
	else N = (44+nb_bits(n&65535/*(n ^ (n<<1))&((1<<17)-1)*/)<<1)<<1;
	
	/*
	 * Process translation & compute nb of cycles of the translated
	 * version (H)
	 */
	ifn(n)	{
		ni = setupinst(insinst(ni),MOVEQ,0,IMM,0,REG,dx);
		sz += 2;
		goto true;
		}
	
	if(ip->opcode==MULS && n<0) {sign = TRUE;n = -n;}

	n &= 0x7fffffff;

	if(n==7)
		{
		for(ds=D0;ds<=D7 && ((ip->live & RM(ds)) || (ds==dx));++ds);
		if(ds<=D7)
			{
			ni = setupinst(insinst(ni),MOVE,LENL,REG,dx,REG,ds);
			ni = setupinst(insinst(ni),ip->opcode==MULU?LSL:ASL,
							LENL,IMM,3,REG,dx);
			ni = setupinst(insinst(ni),SUB,LENL,REG,ds,REG,dx);
			sz += 6;
			goto true;
			}
		}
		
	m = first_nb(n);
	if(nb_bits(n)==1 && 8<=m && m<=15)
		{
		ni = setupinst(insinst(ni),SWAP,0,REG,dx,NONE);
		ni = setupinst(insinst(ni),CLR,LENW,REG,dx,NONE); sz += 4;
		ni = setupinst(insinst(ni),ip->opcode==MULU?LSR:ASR,LENL,
							IMM,16-m,REG,dx); 
		sz += 2;
		goto true;
		}
	
	if(n!=1)
		{
		m = first_nb(n); n >>= m+1;
		PUTSHIFT(ip,m,dx,ni,H,sz)
		if(n!=0)
			{
			for(ds=D0;ds<=D7 && ((ip->live & RM(ds)) || (ds==dx));++ds);
			if(ds>D7) goto fail;
			ni->live |= RM(ds);
			ni = setupinst(insinst(ni),
				MOVE,LENL,REG,dx,REG,ds); H += C_MOVE; sz += 2;
			do	{
				m = first_nb(n)+1; n >>= m;
				PUTSHIFT(ip,m,ds,ni,H,sz)
				ni = setupinst(insinst(ni),
					ADD,LENL,REG,ds,REG,dx); H += C_ADD; sz += 2;
				}
			while(n);
			}
		}

	if(sign)
		{
		ni = setupinst(insinst(ni),NEG,LENL,REG,dx,NONE); sz += 2;
		H += (mc20 || mc40)?2:6;
		}
	
	/*
	 * keep the best version
	 */
	if(size && sz>4) goto fail;
	if(H>=N) 
		{
/*		DBG(printf("(%d>=%d) ",H,N));	/**/
		goto fail;
		}
	
true:	delinst(ip);		/* sucess */
/*	DBG(printf("mul "));	/**/
	return TRUE;

fail:	while(ni!=ip)		/* can't find scratch reg. */
		{INST *ti;
		ti = ni->prev;
		delinst(ni);--s_idel;
		ni = ti;
		}
		
false:	return FALSE;		/* can't optimize */
	}

/*
 *	mulopt() - optimize multiplication
 */
bool mulopt(ip)
	INST *ip;
	{
	/* unavaible for now
	if(ip->flags==LENL) return muloptl(ip);
	*/
	return muloptw(ip);
	}
