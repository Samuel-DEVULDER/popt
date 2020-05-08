/*.
 *	asp68kbis.c - some optimisations from asp68k project.. PartII
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

extern int a_line;

/*.
 * ipeepabis(ip) - check for changes to the instruction 'ip'.
 * return next inst
 */
INST *peepabis(i1)
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
	 *	eor.x #-1,*		=>	not.x *
	 * Not for 060.
	 */
	if(op1==EOR && sm1==IMM && !mc60 &&
	   (i1->src.disp&((i1->flags&LENW)?65535:((i1->flags&LENB)?255:-1)))
	== ((i1->flags&LENW)?65535:((i1->flags&LENB)?255:-1)))
		{
		i1->opcode    = NOT;
		i1->src       = i1->dst;
		i1->dst.amode = NONE;

		DBG(a_line=__LINE__+20000)
		goto true2;
		}
	/*
	 *	lea (Am),Am             =>      <deleted>
	 */
	if(op1==LEA && sm1==REGI && sr1==dr1)
		{
		delinst(i1);

		DBG(a_line=__LINE__+20000)
		goto true2;
		}
	/*
	 *	move optim
	 */

	/*
	 *	move.b	#-1,X		=>	st	X
	 *
	 * UNSAFE: status flags are wrong so test if next instruction
	 * sets them. Not for 68040+.
	 */
	if(!mc60 && !mc40 && (!safe || cc_modified(i2)) && op1==MOVE && 
	   sm1==IMM &&
	   (i1->flags & LENB) && ((i1->src.disp&255) == 255))
		{
		i1->opcode    = ST;
		i1->src       = i1->dst;
		i1->dst.amode = NONE;

		DBG(a_line=__LINE__+20000)
		goto true2;
		}
	/*
	 *	move.l	#n,-(sp)        =>      pea     n.W
	 *
	 * Where n is a valid displacement. Not for 68040+.
	 */
	if(!mc60 && !mc40 && op1==MOVE && sm1==IMM && dm1==(REGID|DEC) && 
	   dr1==SP &&
	   DOK(i1->src.disp) && (i1->flags & LENL))
		{
		setupinst(i1,PEA,0,ABS|XWORD,i1->src.disp,NONE);
		DBG(a_line=__LINE__+20000)
		goto true2;
		}
	/*
	 *	move.l	#n,Dx		=>	moveq	#-128,Dx
	 *					subq.l	#n+128,Dx
	 *
	 * Where -136<=n<=-129. Not for 68040.
	 */
	if(!mc40 && op1==MOVE && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->flags & LENL) && (-136<=i1->src.disp) && (i1->src.disp<=-129))
		{ INST *ni;
		ni = setupinst(inspreinst(i1),MOVEQ,0,IMM,-128,REG,dr1);
		i1->opcode     = SUBQ;
		i1->src.disp  += 128;
		DBG(a_line=__LINE__+20000)
		goto true2;
		}

	/*
	 *	move.l	#n,Dx		=>	moveq	#m,Dx
	 *					not.b	Dx
	 *
	 * Where n=255-m (128>m>=0). Not for 68040.
	 */
	if(!mc40 && op1==MOVE && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->flags & LENL) && (128<=i1->src.disp) &&
	   (i1->src.disp<=255))
		{ INST *ni;
		i1->opcode     = MOVEQ;
		i1->flags      = 0;
		i1->src.disp   = 255-i1->src.disp;
		ni = setupinst(insinst(i1),NOT,LENB,REG,dr1,NONE);
		DBG(a_line=__LINE__+20000)
		goto true2;
		}

	/*
	 *	move.l	#n,Dx		=>	moveq	#m,Dx
	 *					not.w	Dx
	 *
	 * Where 65534 <= n <= 65408 , m = 65535-n. Not for 68040.
	 */
	if(!mc40 && op1==MOVE && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->flags & LENL) && (65534<=i1->src.disp) &&
	   (i1->src.disp<=65408))
		{ INST *ni;
		i1->opcode     = MOVEQ;
		i1->flags      = 0;
		i1->src.disp   = 65535-i1->src.disp;
		ni = setupinst(insinst(i1),NOT,LENW,REG,dr1,NONE);
		DBG(a_line=__LINE__+20000)
		goto true2;
		}
	/*
	 *	move.l	#n,Dx		=>	moveq	#m,Dx
	 *					not.w	Dx
	 *
	 * Where -65409 <= n <= -65536, m = 65535+n. Not for 68040.
	 */
	if(!mc40 && op1==MOVE && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->flags & LENL) && (-65409<=i1->src.disp) &&
	   (i1->src.disp<=-65536))
		{ INST *ni;
		i1->opcode     = MOVEQ;
		i1->flags      = 0;
		i1->src.disp   = 65535+i1->src.disp;
		ni = setupinst(insinst(i1),NOT,LENW,REG,dr1,NONE);
		DBG(a_line=__LINE__+20000)
		goto true2;
		}
	/*
	 *	move.l	#n,Dx		=>	moveq	#m,Dx
	 *					swap	Dx
	 *
	 * Where n=m*65536 (-128<=m<=127). Not for 68040.
	 */
	if(!mc40 && op1==MOVE && sm1==IMM && dm1==REG && ISD(dr1) &&
	   (i1->flags & LENL) && (i1->src.disp&0xffff0000)==i1->src.disp &&
	   (-128<=(i1->src.disp>>16)) && ((i1->src.disp>>16)<=127))
		{ INST *ni;
		i1->opcode     = MOVEQ;
		i1->flags      = 0;
		i1->src.disp >>= 16;
		ni = insinst(i1);
		ni->opcode     = SWAP;
		ni->src.amode  = REG;
		ni->src.areg   = dr1;
		DBG(a_line=__LINE__+20000)
		goto true2;
		}

	/*
	 *	Other moves are very complicated and not implemented.
	 *	Those uses thing like
	 *		move.l	#n,dx	=>	moveq	#m,dx
	 *					bchg	dx,dx
	 *	or
	 *				=>	moveq	#m,dx
	 *					lsl.l	#p,dx
	 *	I think they are not worth.
	 */

	/*
	 *	mul optim - separate module mulopt.c ...
	 *	UNSAFE...
	 */
	if((!safe || cc_modified(i2)) && (op1==MULS || op1==MULU))
		if(mulopt(i1)) {DBG(a_line=__LINE__+20000) goto true2;}

	/*
	 *	neg.x	Rx		=>	<deleted>
	 *	sub.x	Rx,Ry			add.x	Rx,Ry
	 *
	 * add or sub. Where Rx is dead
	 */
	if(op1==NEG && sm1==REG && (OP_ADD(op2) || OP_SUB(op2)) &&
	   sm2==REG && dm2==REG && sr1==sr2 && !(i2->live & RM(sr1)) &&
	   i1->flags == i2->flags && !i2->label)
		{
		i2->opcode = OP_SUB(op2)?ADD:SUB;
		delinst(i1);
		DBG(a_line=__LINE__+20000)
		goto true2;
		}
	/*
	 *	or.l #n,dx			=> bset.l #b,dx
	 *
	 * Where n = 2^b (only 1 bit on). Not for 68020+.
	 */
	if(!mc60 && !mc40 && !mc20 &&
	   op1==AND && (i1->flags&LENL) && sm1==IMM && dm1==REG &&
	   ISD(dr1) && nb_bits(i1->src.disp)==1)
		{
		i1->opcode   = BSET;
		i1->src.disp = first_nb(i1->src.disp);
		DBG(a_line=__LINE__+20000)
		goto true2;
		}

	return NULL;

true:	return i1->next;			/* return when deletion */
true2:	return i2;				/* std return */
	}
