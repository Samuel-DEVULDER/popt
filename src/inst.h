/*
 *	inst.h - Misc. define's, etc. for instruction parsing.
 *
 *	(c) by Samuel DEVULDER
 */

struct	opnd {
	unsigned short	amode;	/* addressing mode used */
/*unsigned*/ 	char	areg;	/* primary register */
/*unsigned*/ 	char	ireg;	/* index register, if applicable */
	union {
		long	i_disp;		/* also used for immediate data */
		char	*i_astr;	/* pointer to any symbol present */
	} i_data;
};
#define	disp	i_data.i_disp
#define	astr	i_data.i_astr

/*
 * is_astr(m)	macro to determine, based on the amode field, whether
 *		the 'astr' field of the structure is active.
 */
#define	is_astr(m)	(((m)&SYMB) || (m) == STRNG)

/*
 * Addressing modes (in 'amode')
 */
#define	NONE	0		/* operand unused */
#define	REG	1		/* register direct */
#define	IMM	2		/* immediate */
#define	ABS	3		/* absolute */
#define	REGI	4		/* reg. indirect */
#define	REGID	5		/* reg. indirect, w/ displacement */
#define	REGIDX	6		/* reg. indirect, w/ displacement & index */
#define	PCD	7		/* PC relative, w/ displacement */
#define	PCDX	8		/* PC relative, w/ displacement & index */
#define	MSK	9		/* D0-D3/A0-A7 ... */
#define	STRNG	10		/* strange mode... */

#define	XLONG	0x010		/* long index register used */
#define	XWORD	0x010		/* (ABS).w */
#define	SYMB	0x020		/* symbol used, not constant */
#define	INC	0x040		/* auto-increment */
#define	DEC	0x080		/* auto-decrement */
#define	X2	0x100		/* Dn*2 */
#define	X4	0x200		/* Dn*4 */
#define	X8	0x400		/* Dn*8 */

#define	MMASK	0x00f		/* mode mask */
#define	FMASK	0xff0		/* flag mask */

#define	M(x)	((x) & MMASK)
#define	F(x)	((x) & FMASK)

/*
 * Registers
 */

#define	FIRSTREG	0
#define	A0		0
#define	A1		1
#define	A2		2
#define	A3		3
#define	A4		4
#define	A5		5
#define	A6		6
#define	A7		7
#define	SP		7	/* alias for A7 */
#define	D0		8
#define	D1		9
#define	D2		10
#define	D3		11
#define	D4		12
#define	D5		13
#define	D6		14
#define	D7		15
#define	LASTREG		15

#define	ALLREGS		65535
#define	ALLAREGS	255
#define	ALLDREGS	(255<<8)

#define	ISD(x)	((x) >= D0 && (x) <= D7)	/* is 'x' a  D reg. */
#define	ISA(x)	((x) >= A0 && (x) <= A7)	/* is 'x' an A reg. */
#define	RM(x)	(1 << (x))			/* form a register mask */

/*
 * DOK(x) - evaluates TRUE if 'x' is okay for a displacement value.
 *
 * 'x' must be of type "long"
 */
#define	DOK(x)	(((x) >= -32768L) && ((x) <= 32767L))

/*
 * D8OK(x) - like DOK but for 8-bit displacements
 */
#define	D8OK(x)	(((x) >= -128) && ((x) <= 127))

struct	inst {
	unsigned char	opcode,		/* type of instruction */
			flags;		/* length, etc. */
	struct	opnd	src, dst;	/* optional operands */
	char		*label;		/* label */
	char		*rest;		/* comments */
	unsigned short	nb_hat,		/* number of '^' */
			refs,sets,	/* which reg is ref or set by inst */
			live,		/* which reg is alive after inst */ 
			lineno;		/* lineno of inst in sourcefile */
	struct	inst	*next;		/* next & previous line in file */
	struct	inst	*prev;
};

/*
 * Instruction flags
 */

#define	LENB	0x01
#define	LENW	0x02
#define	LENL	0x04
