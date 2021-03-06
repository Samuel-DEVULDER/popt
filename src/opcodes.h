/*
 *	opcodes.h - Defines for the instructions
 *
 *	(c) by Samuel DEVULDER
 */

enum	{
	EQUAL = 0,
	ABCD,
	ADD,
	ADDA,
	ADDI,
	ADDQ,
	ADDX,
	AND,
	ANDI,
	ASL,
	ASR,
	BCC,
	BCHG,
	BCLR,
	BCS,
	BEQ,
	BGE,
	BGT,
	BHI,
	BLE,
	BLS,
	BLT,
	BMI,
	BNE,
	BPL,
	BRA,
	BSET,
	BSR,
	BSS,
	BTST,
	BVC,
	BVS,
	CHK,
	CLR,
	CMP,
	CMPA,
	CMPI,
	CMPM,
	CNOP,
	CODE,
	CSEG,
	DATA,
	DBCC,
	DBCS,
	DBEQ,
	DBF,
	DBGE,
	DBGT,
	DBHI,
	DBLE,
	DBLS,
	DBLT,
	DBMI,
	DBNE,
	DBPL,
	DBRA,
	DBT,
	DBVC,
	DBVS,
	DC,
	DCB,
	DIVS,
	DIVU,
	DS,
	DSEG,
	END,
	ENDC,
	ENDIF,
	EOR,
	EORI,
	EQU,
	EQUR,
	EVEN,
	EXG,
	EXT,
	FAR,
	GLOBAL,
	IDNT,
	IFC,
	IFD,
	IFEQ,
	IFGE,
	IFGT,
	IFLE,
	IFLT,
	IFNC,
	IFND,
	IFNE,
	ILLEGAL,
	INCLUDE,
	JMP,
	JSR,
	LEA,
	LINK,
	LIST,
	LSL,
	LSR,
	MACRO,
	MOVE,
	MOVEA,
	MOVEM,
	MOVEP,
	MOVEQ,
	MULS,
	MULU,
	NBCD,
	NEAR,
	NEG,
	NEGX,
	NOL,
	NOLIST,
	NOP,
	NOT,
	OR,
	ORG,
	ORI,
	PAGE,
	PEA,
	PUBLIC,
	ReG,
	RESET,
	ROL,
	ROR,
	RORG,
	ROXL,
	ROXR,
	RTE,
	RTR,
	RTS,
	SBCD,
	SCC,
	SCS,
	SECTION,
	SEQ,
	SeT,
	SF,
	SGE,
	SGT,
	SHI,
	SLE,
	SLS,
	SLT,
	SMI,
	SNE,
	SPC,
	SPL,
	ST,
	STOP,
	SUB,
	SUBA,
	SUBI,
	SUBQ,
	SUBX,
	SVC,
	SVS,
	SWAP,
	TAS,
	TITLE,
	TRAP,
	TRAPV,
	TST,
	UNLK,
	XDEF,
	XREF,
	PROCSTART,
	PROCEND,
	EXTB,

	UNKWN,
	NO_INST
	};
