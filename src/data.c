/*
 *	data.c - Routines for data flow analysis of a single instruction.
 *
 *	(c) by Samuel DEVULDER
 */

#include "popt.h"

/*
 * idata
 *
 * For each instruction, we have some global information, as well
 * as flags indicating what the instruction does with its operands.
 * We need to know if each operand is set and/or referenced. If the
 * instruction has side-effects not directly related to its operands,
 * we need to know that as well, so "special case" code can deal with
 * that as well.
 */
struct	idata	{

	char	iflag;		/* flags regarding the entire instruction */
#define		SIDE	0x01	/* inst. has side-effects */
#define		CC	0x02	/* inst. munges condition codes */
#define		PSDO	0x04	/* pseudo inst */

	char	op1f;		/* flags for the first and second operands */
	char	op2f;
#define		SET	0x01	/* operand is set */
#define		REF	0x02	/* operand is referenced */

} idata[] =
{
	{ PSDO,		0,		0 },		/* EQUAL */
	{ CC,		REF,		REF|SET },	/* ABCD */
	{ CC,		REF,		REF|SET },	/* ADD */
	{ 0,		REF,		REF|SET },	/* ADDA */
	{ CC,		REF,		REF|SET },	/* ADDI */
	{ CC,		REF,		REF|SET },	/* ADDQ */
	{ CC,		REF,		REF|SET },	/* ADDX */
	{ CC,		REF,		REF|SET },	/* AND */
	{ CC,		REF,		REF|SET },	/* ANDI */
	{ CC,		REF,		REF|SET },	/* ASL */
	{ CC,		REF,		REF|SET },	/* ASR */
	{ 0,		REF,		0 },		/* BCC */
	{ CC,		REF,		REF|SET },	/* BCHG */
	{ CC,		REF,		REF|SET },	/* BCLR */
	{ 0,		REF,		0 },		/* BCS */
	{ 0,		REF,		0 },		/* BEQ */
	{ 0,		REF,		0 },		/* BGE */
	{ 0,		REF,		0 },		/* BGT */
	{ 0,		REF,		0 },		/* BHI */
	{ 0,		REF,		0 },		/* BLE */
	{ 0,		REF,		0 },		/* BLS */
	{ 0,		REF,		0 },		/* BLT */
	{ 0,		REF,		0 },		/* BMI */
	{ 0,		REF,		0 },		/* BNE */
	{ 0,		REF,		0 },		/* BPL */
	{ 0,		REF,		0 },		/* BRA */
	{ CC,		REF|SET,	0 },		/* BSET */
	{ SIDE,		REF,		0 },		/* BSR */
	{ PSDO,		0,		0 },		/* BSS */
	{ CC,		REF,		REF },		/* BTST */
	{ 0,		REF,		0 },		/* BVC */
	{ 0,		REF,		0 },		/* BVS */
	{ CC,		REF,		REF },		/* CHK */
	{ CC,		SET,		0 },		/* CLR */
	{ CC,		REF,		REF },		/* CMP */
	{ CC,		REF,		REF },		/* CMPA */
	{ CC,		REF,		REF },		/* CMPI */
	{ CC,		REF,		REF },		/* CMPM */
	{ 0,		0,		0 },		/* CNOP */
	{ PSDO,		0,		0 },		/* CODE */
	{ PSDO,		0,		0 },		/* CSEG */
	{ PSDO,		0,		0 },		/* DATA */
	{ CC,		REF|SET,	REF },		/* DBCC */
	{ CC,		REF|SET,	REF },		/* DBCS */
	{ CC,		REF|SET,	REF },		/* DBEQ */
	{ CC,		REF|SET,	REF },		/* DBF */
	{ CC,		REF|SET,	REF },		/* DBGE */
	{ CC,		REF|SET,	REF },		/* DBGT */
	{ CC,		REF|SET,	REF },		/* DBHI */
	{ CC,		REF|SET,	REF },		/* DBLE */
	{ CC,		REF|SET,	REF },		/* DBLS */
	{ CC,		REF|SET,	REF },		/* DBLT */
	{ CC,		REF|SET,	REF },		/* DBMI */
	{ CC,		REF|SET,	REF },		/* DBNE */
	{ CC,		REF|SET,	REF },		/* DBPL */
	{ CC,		REF|SET,	REF },		/* DBRA */
	{ CC,		REF|SET,	REF },		/* DBT */
	{ CC,		REF|SET,	REF },		/* DBVC */
	{ CC,		REF|SET,	REF },		/* DBVS */
	{ PSDO,		0,		0 },		/* DC */
	{ PSDO,		0,		0 },		/* DCB */
	{ CC,		REF,		REF|SET },	/* DIVS */
	{ CC,		REF,		REF|SET },	/* DIVU */
	{ PSDO,		0,		0 },		/* DS */
	{ PSDO,		0,		0 },		/* DSEG */
	{ PSDO,		0,		0 },		/* END */
	{ PSDO,		0,		0 },		/* ENDC */
	{ PSDO,		0,		0 },		/* ENDIF */
	{ CC,		REF,		REF|SET },	/* EOR */
	{ CC,		REF,		REF|SET },	/* EORI */
	{ PSDO,		0,		0 },		/* EQU */
	{ PSDO,		0,		0 },		/* EQUR */
	{ PSDO,		0,		0 },		/* EVE */
	{ 0,		REF|SET,	REF|SET },	/* EXG */
	{ CC,		REF|SET,	0 },		/* EXT */
	{ PSDO,		0,		0 },		/* FAR */
	{ PSDO,		0,		0 },		/* GLOBAL */
	{ PSDO,		0,		0 },		/* IDNT */
	{ PSDO,		0,		0 },		/* IFC */
	{ PSDO,		0,		0 },		/* IFD */
	{ PSDO,		0,		0 },		/* IFEQ */
	{ PSDO,		0,		0 },		/* IFGE */
	{ PSDO,		0,		0 },		/* IFGT */
	{ PSDO,		0,		0 },		/* IFLE */
	{ PSDO,		0,		0 },		/* IFLT */
	{ PSDO,		0,		0 },		/* IFNC */
	{ PSDO,		0,		0 },		/* IFND */
	{ PSDO,		0,		0 },		/* IFNE */
	{ PSDO,		0,		0 },		/* ILLEGAL */
	{ PSDO,		0,		0 },		/* INCLUDE */
	{ SIDE,		REF,		0 },		/* JMP */
	{ SIDE,		REF,		0 },		/* JSR */
	{ 0,		REF,		SET },		/* LEA */
	{ SIDE,		REF|SET,	REF },		/* LINK */
	{ PSDO,		0,		0 },		/* LIST */
	{ CC,		REF,		REF|SET },	/* LSL */
	{ CC,		REF,		REF|SET },	/* LSR */
	{ PSDO,		0,		0 },		/* MACRO */
	{ CC,		REF,		SET },		/* MOVE */
	{ 0,		REF,		SET },		/* MOVEA */
	{ 0,		REF,		SET },		/* MOVEM */
	{ 0,		REF,		SET },		/* MOVEP */
	{ CC,		REF,		SET },		/* MOVEQ */
	{ CC,		REF,		REF|SET },	/* MULS */
	{ CC,		REF,		REF|SET },	/* MULU */
	{ CC,		REF|SET,	0 },		/* NBCD */
	{ PSDO,		0,		0 },		/* NEAR */
	{ CC,		REF|SET,	0 },		/* NEG */
	{ CC,		REF|SET,	0 },		/* NEGX */
	{ PSDO,		0,		0 },		/* NOL */
	{ PSDO,		0,		0 },		/* NOLIST */
	{ 0,		0,		0 },		/* NOP */
	{ CC,		REF|SET,	0 },		/* NOT */
	{ CC,		REF,		REF|SET },	/* OR */
	{ PSDO,		0,		0 },		/* ORG */
	{ CC,		REF,		REF|SET },	/* ORI */
	{ PSDO,		0,		0 },		/* PAGE */
	{ SIDE,		REF,		0 },		/* PEA */
	{ PSDO,		0,		0 },		/* PUBLIC */
	{ PSDO,		0,		0 },		/* REG */
	{ 0,		0,		0 },		/* RESET */
	{ CC,		REF,		REF|SET },	/* ROL */
	{ CC,		REF,		REF|SET },	/* ROR */
	{ PSDO,		0,		0 },		/* RORG */
	{ CC,		REF,		REF|SET },	/* ROXL */
	{ CC,		REF,		REF|SET },	/* ROXR */
	{ SIDE|CC,	0,		0 },		/* RTE */
	{ SIDE|CC,	0,		0 },		/* RTR */
	{ SIDE,		0,		0 },		/* RTS */
	{ CC,		REF,		REF|SET },	/* SBCD */
	{ 0,		REF|SET,	0 },		/* SCC */
	{ 0,		REF|SET,	0 },		/* SCS */
	{ PSDO,		0,		0 },		/* SECTION */
	{ 0,		REF|SET,	0 },		/* SEQ */
	{ PSDO,		0,		0 },		/* SET */
	{ 0,		REF|SET,	0 },		/* SF */
	{ 0,		REF|SET,	0 },		/* SGE */
	{ 0,		REF|SET,	0 },		/* SGT */
	{ 0,		REF|SET,	0 },		/* SHI */
	{ 0,		REF|SET,	0 },		/* SLE */
	{ 0,		REF|SET,	0 },		/* SLS */
	{ 0,		REF|SET,	0 },		/* SLT */
	{ 0,		REF|SET,	0 },		/* SMI */
	{ 0,		REF|SET,	0 },		/* SNE */
	{ PSDO,		0,		0 },		/* SPC */
	{ 0,		REF|SET,	0 },		/* SPL */
	{ 0,		REF|SET,	0 },		/* ST */
	{ CC,		REF,		0 },		/* STOP */
	{ CC,		REF,		REF|SET },	/* SUB */
	{ 0,		REF,		REF|SET },	/* SUBA */
	{ CC,		REF,		REF|SET },	/* SUBI */
	{ CC,		REF,		REF|SET },	/* SUBQ */
	{ CC,		REF,		REF|SET },	/* SUBX */
	{ 0,		SET,		0 },		/* SVC */
	{ 0,		SET,		0 },		/* SVS */
	{ CC,		REF|SET,	0 },		/* SWAP */
	{ CC,		REF|SET,	0 },		/* TAS */
	{ PSDO,		0,		0 },		/* TITLE */
	{ 0,		REF,		0 },		/* TRAP */
	{ 0,		0,		0 },		/* TRAPV */
	{ CC,		REF,		0 },		/* TST */
	{ SIDE,		REF|SET,	0 },		/* UNLK */
	{ PSDO,		0,		0 },		/* XDEF */
	{ PSDO,		0,		0 },		/* XREF */
	{ PSDO,		0,		0 },		/* PROCSTART */
	{ PSDO,		0,		0 },		/* PROCEND */
	{ CC,		REF|SET,	0 },		/* EXTB */

	{ 0,		0,		0 },		/* UNKWN */
	{ 0,		0,		0 },		/* NO_INST */
};

/*
 * chkset(op) - check to see if operand 'op' sets a register
 *
 * This given operand is set by an instruction. Depending on the
 * addressing mode used, this may set a register. If so, return
 * an appropriate mask. This only happens with register direct
 * addressing.
 */
static int chkset(op)
	register struct	opnd	*op;
	{
	if(op->amode == MSK) return op->disp;
	switch(M(op->amode))
		{
		case STRNG:
		return ALLREGS;

		case REG:
		return RM(op->areg);

		case REGI:
		return (op->amode & (INC|DEC))?RM(op->areg):0;

		default:
		return 0;
		}
	}

/*
 * chkref(op) - check to see if operand 'op' references a register
 *
 * Checks for register references in source or destination
 * operands, since they can occur in either.
 */
static int chkref(ip, op, is_src)
	INST *ip;
	register struct	opnd	*op;
	register bool	is_src;		/* is the operand a source? */
	{
	if(op->amode == MSK) return is_src?op->disp:0;
	switch (M(op->amode))
		{
		case STRNG:
		return ALLREGS;

		case NONE:
		case IMM:
		case ABS:
		case PCD:
		return 0;

		case REG:
		if(is_src) return RM(op->areg); else return 0;

		case REGI:
		case REGID:
		return RM(op->areg);

		case REGIDX:
		return (RM(op->areg) | RM(op->ireg));

		case PCDX:
		return RM(op->ireg);

		default:
		fprintf(stderr, "%s: chkref() - illegal mode %d\n",
			ProgName, M(op->amode));
		dumpline(ip);
		exit(1);
		}
	}

/*
 * chkside(ip, type) - check for side-effects of 'ip'
 *
 * Return a mask of registers set or referenced (depending on 'type')
 * by the given instruction. For example, "pea" sets and references
 * the stack pointer.
 */
static int chkside(ip, type)
	register INST	*ip;
	register int	type;
	{
	switch (ip->opcode)
		{
		case PEA:		/* refs/sets the stack pointer */
		return RM(SP);

		case LINK:		/* refs/sets SP */
		return RM(SP);

		case UNLK:
		return (type==REF)?0:RM(SP);


		case JMP:
		{INST *ni;
		/*
		 *	move.l	(sp)+,An
		 *	jmp	(An)
		 *
		 * must be treated as a RTS for live/dead/used regs...
		 */
		ifn(ni=ip->prev) return 0;
		ifn(M(ip->src.amode)==REGI) return 0;
		ifn(ni->opcode==MOVE && ni->flags==LENL && 
		    ni->src.amode==(REGI|INC) && (ni->src.areg==SP) &&
		    ni->dst.amode==REG && ISA(ni->dst.areg) && 
		    ni->dst.areg==ip->src.areg) return 0;
		return (type==REF)?used_rts|RM(SP):0;
		}

		case RTE:
		case RTS:
		case RTR:
		return (type==REF)?used_rts|RM(SP):RM(SP);

		case JSR:
		case BSR:
		/*
		 *	Hack for non-std func call: mulu.l is simulated
		 *	by a call to __mulu. We assume a call to __something
		 *	refs d0-d1 and sets d0-d1. This can work may be
		 *	for simple IEEE floats. I don't kwon for double
		 *	float (maybe should I mark d2-d3 as beeing used
		 *	also ?).
		 */
		if(is_astr(ip->src.amode) && 
		   (ip->src.astr[0]=='_' && ip->src.astr[1]=='_'))
		   	{
			return (type==REF)?(RM(SP)|cr_regs|RM(D0)|RM(D1)):
					   (RM(SP)|cs_regs|RM(D0)|RM(D1));
		   	}
		/*
		 *	For DICE and SAS, we say @func uses d0/d1/a0/a1
		 */
		if(is_astr(ip->src.amode) && ip->src.astr[0]=='@')
		   	{
			return (type==REF)?(RM(SP)|cr_regs|RM(D0)|RM(D1)|RM(A0)|RM(A1)):
					   (RM(SP)|cs_regs|RM(D0)|RM(D1));
		   	}
		/*
		 *	Same for .something# 
		 */
		if(is_astr(ip->src.amode) && 
		   ip->src.astr[0]=='.' && 
		   ip->src.astr[strlen(ip->src.astr)-1]=='#')
		   	{
			return (type==REF)?(RM(SP)|cr_regs|RM(D0)|RM(D1)):
					   (RM(SP)|cs_regs|RM(D0)|RM(D1));
		   	}
		return (type==REF)?(RM(SP)|cr_regs):(RM(SP)|cs_regs);

		default:
		fprintf(stderr, "%s: chkside() - Unknown opcode %d\n",
			ProgName,ip->opcode);
		dumpline(ip);
		exit(1);
		}
	}

/*
 * cc_modified(ip) - return true if 'ip' modifies CC
 */
bool cc_modified(ip)
	register INST	*ip;
	{
	ifn(ip) return TRUE;
	return (idata[ip->opcode].iflag & CC)?TRUE:FALSE;
	}

/*
 * reg_set(ip) - return mask of regs set by 'ip'
 */
unsigned short reg_set(ip)
	register INST	*ip;
	{
	int	mask = 0;	/* build up a register mask */
	int	data;
	if(ip->opcode == UNKWN) return ALLREGS;
	if(idata[ip->opcode].iflag & PSDO) return 0;
	if(idata[ip->opcode].op1f & SET) mask |= chkset(&ip->src);
	if(idata[ip->opcode].op1f & REF) if((ip->src.amode & (INC|DEC)))  mask |= RM(ip->src.areg);
	if(idata[ip->opcode].op2f & SET) mask |= chkset(&ip->dst);
	if(idata[ip->opcode].op2f & REF) if((ip->dst.amode & (INC|DEC)))  mask |= RM(ip->dst.areg);
	if(idata[ip->opcode].iflag & SIDE) mask |= chkside(ip, SET);
	return mask;
	}

/*
 * reg_ref(ip) - return mask of regs referenced by 'ip'
 */
unsigned short reg_ref(ip)
	register INST	*ip;
	{
	int	mask = 0;	/* build up a register mask */
	INST	*ni;

	if(ip->opcode == UNKWN) return ALLREGS;
	if(idata[ip->opcode].iflag & PSDO) return 0;
	mask |= chkref(ip, &ip->src, idata[ip->opcode].op1f & REF);
	mask |= chkref(ip, &ip->dst, idata[ip->opcode].op2f & REF);
	if(idata[ip->opcode].iflag & SIDE) mask |= chkside(ip, REF);
	/*
	 * HACK NOTE: we say that move.w X,dx refs dx (indeed dx.h) if the
	 * previous instruction is a long move on dx but not if next inst is
	 * ext.l dx. For safe option, move.w X,dx always refs dx...
	 */
	ifn(ip->opcode==MOVE && (ip->flags&(LENW|LENB)) && 
	    ip->dst.amode==REG && ISD(ip->dst.areg)) goto end;
	if(safe) {mask |= RM(ip->dst.areg);goto end;}
	ni=ip->next;
	if(ni->opcode==EXT && ni->flags==LENL && ip->flags==LENW &&
	   ni->src.areg==ip->dst.areg) goto end;
	ni=ip->prev;
	if(OP_MOVE(ni->opcode) && ((ni->flags?ni->flags:4)>ip->flags) && 
	   ni->dst.amode==REG && ni->dst.areg==ip->dst.areg) 
		mask |= RM(ip->dst.areg);
	if(ni->opcode==CLR && (ni->flags&LENL) && 
	   ni->src.amode==REG && ni->src.areg==ip->dst.areg) 
		mask |= RM(ip->dst.areg);
end:	return mask;
	}

/*
 * sets(ip, reg) - is 'reg' set by the instruction 'ip'?
 */
bool sets(ip, reg)
	INST	*ip;
	int	reg;
	{
	return (ip->sets & RM(reg)) != 0;
	}

/*
 * refs(ip, reg) - is 'reg' referenced by the instruction 'ip'?
 */
bool refs(ip, reg)
	INST	*ip;
	int	reg;
	{
	return (ip->refs & RM(reg)) != 0;
	}

/*
 * uses(ip, reg) - is 'reg' used by the instruction 'ip'?
 */
bool uses(ip, reg)
	INST	*ip;
	int	reg;
	{
	return sets(ip, reg) || refs(ip, reg);
	}
	
/*
 * uses_slow(ip, reg) - is 'reg' used by the instruction 'ip'? Slow version.
 *	(used when health() not already done !)
 */
bool uses_slow(ip, reg)
	INST	*ip;
	int	reg;
	{
	return (reg_set(ip) & RM(reg)) || 
	       (reg_ref(ip) & RM(reg));
	}
	
/*
 * uses_cc(op) - is op a code that uses cc
 */
bool uses_cc(op)
	int op;
	{
	switch(op)
		{
		case BCC: case BCS: case BEQ: case BGE:
		case BGT: case BHI: case BLE: case BLS:	
		case BLT: case BMI: case BNE: case BPL:	
		case BVC: case BVS:

		case DBCC: case DBCS: case DBEQ: case DBF:
		case DBGE: case DBGT: case DBHI: case DBLE:
		case DBLS: case DBLT: case DBMI: case DBNE:
		case DBPL: case DBRA: case DBT:  case DBVC:
		case DBVS:

		case ADDX: case SUBX: case NEGX: 
		case SCC:  case SCS:  case SEQ: 
		case SGE:  case SGT:  case SHI:  case SLE:
		case SLS:  case SLT:  case SMI:  case SNE:
		case SPL:  case SVC:  case SVS:  case SWAP:
		case SF:   case ST:
		
		case ROXL: case ROXR:
		return TRUE;
		
		default: return FALSE;
		}
	}

/*
 * is_cc_dead(ip) - is cc dead at ip
 */
bool is_cc_dead(ip)
	INST *ip;
	{
	ifn(ip = ip->next)     return TRUE;
	if(uses_cc(ip->opcode)) return FALSE;
	if(cc_modified(ip))    return TRUE;
	return safe?FALSE:TRUE;
	}

/*
 * is_real_inst(op) - is op a real instruction 
 */
bool is_real_inst(op)
	int op;
	{
	return !(idata[op].iflag & PSDO);
	}
