/*
 *	popt.h
 *
 *	(c) by Samuel DEVULDER
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "inst.h"
#include "opcodes.h"

#define	DEBUG		1	/* enable debug code */

#ifdef	DEBUG
#define	DBG(x)		if (debug) { x; }
#else
#define	DBG(x)
#endif

#ifndef	void			/* for the PDC compiler */
#define	void		int
#endif

#ifndef __GNUC__
typedef unsigned short 	ushort;
#else
#define register
#endif

/*
 * Usefull defines
 */
#define	ifn(x)		if(!(x))
#define	NEW(x)		((x)=(void*)alloc(sizeof(*(x))))
#define DELETE(x)	((void*)(free(x),(x)=NULL))

/*
 * Basic defines and declarations for the optimizer.
 */

typedef	int	bool;

#ifndef	FALSE
#define	FALSE	0
#define	TRUE	1
#endif

#define	OP_MOVE(op)	(op==MOVE || op==MOVEA || op==MOVEQ)
#define	OP_ADD(op)	(op==ADD  || op==ADDI  || op==ADDA || op==ADDQ)
#define	OP_SUB(op)	(op==SUB  || op==SUBI  || op==SUBA || op==SUBQ)
#define	OP_OR(op)	(op==OR   || op==ORI)
#define	OP_AND(op)	(op==AND  || op==ANDI)
#define	OP_EOR(op)	(op==EOR  || op==EORI)
#define	OP_CMP(op)	(op==CMP  || op==CMPA || op==CMPI || op==CMPM)
#define	OP_ARITHM(op)	(OP_ADD(op) || OP_SUB(op) || OP_AND(op) ||\
			 OP_OR(op) || OP_EOR(op) || op==LSL ||\
			 op==LSR || op==ASR || op==ASL)
/*
 * Basic Block:
 *
 * References a linked list of instructions that make up the block.
 * Each block can be exited via one of two branches, which are
 * represented by pointers to two other blocks, or null.
 */
typedef	struct inst	INST;
extern	INST	*head;

/*
 * Global data
 */
extern	char	*ifilename;		/* Name of source-file processed  */
extern	char	*ProgName;		/* Name of the program (should    */
					/* be popt) */
extern  char	IDstring[];		/* IDentity string */
extern	char	*opnames[];		/* mnemonics for the instructions */

/*
 * Option flags set in main
 */
#ifdef	DEBUG
extern	bool	debug;
#endif
extern	bool	safe;		/* only safe operations performed */
extern	bool	size;		/* optimize if size is reduced */
extern	bool	mc60;		/* optimize for 68060 */
extern	bool	mc40;		/* optimize for 68040 */
extern	bool	mc20;		/* optimize for 68020 */
extern	bool	newsn;
extern	bool	keeplbl;
extern	bool	keepcomment;
extern	bool	slflag;
extern	bool	nowarning;
extern	bool	nostack;
extern	bool	newinsts;
extern	bool	last_pass;

extern	int	cr_regs;
extern	int	cs_regs;
extern	int	used_rts;

extern	int	s_bdel;		/* branches deleted */
extern	int	s_brev;		/* branches reversed */
extern	int	s_link;		/* link/unlink deleted */
extern	int	s_labmerge;	/* label merged */
extern	int	s_peep1;	/* 1 instruction peephole changes */
extern	int	s_peep2;	/* 2 instruction peephole changes */
extern	int	s_peep3;	/* 3 instruction peephole changes */
extern	int	s_peep4;	/* 4 instruction peephole changes */
extern	int	s_asp68k;	/* asp68k project optimizations */
extern	int	s_mc60;		/* 68060 optimizations */
extern	int	s_mc40;		/* 68040 optimizations */
extern	int	s_mc20;		/* 68020 optimizations */
extern	int	s_idel;		/* instructions deleted */
extern	int	s_jsr;		/* bsr/jsr modified */
extern	int	s_movem;	/* stripped movem */
extern	int	s_stack;	/* merged stack op. */

/*
 * These are set after calling readline.
 */
extern	long	line_no;		/* Number of the last line */
extern	char	buf_line[];		/* Last line read */

/*
 * Function declarations
 */
#include "proto.h"
