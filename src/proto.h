/*
 *	proto.h - Function declarations.
 *
 *	(c) by Samuel DEVULDER
 */

#ifndef _
#	ifdef	__STDC__
#		define	_(x)    x
#	else
#		define	_(x)    ()
#	endif
#endif

/*
 * branchopt.c
 */
extern	void	branchopt	_(());

/*
 * branchopt2.c
 */
extern	bool	islabelused	_((char *));
extern	int	bcomp		_((int));
extern	int	scomp		_((int));
extern	int	binv		_((int));
extern	INST	*find_label	_((char *));
extern	void	cleanup 	_(());
extern	void	del_unreach	_(());

/*
 * data.c
 */
extern	ushort	reg_ref 	_((INST *));
extern	ushort	reg_set 	_((INST *));
extern	int	cc_modified	_((INST *));
extern	bool	sets		_((INST *,int));
extern	bool	refs		_((INST *,int));
extern	bool	uses		_((INST *,int));
extern	bool	uses_slow	_((INST *,int));
extern	bool	uses_cc 	_((int));
extern	bool	is_real_inst	_((int));
extern	bool	is_cc_dead	_((INST *));

/*
 * health.c
 */
extern	bool	chkxref 	_((char *));
extern	void	do_health	_(());
extern	int	isbranch	_((int));
extern	int	db_cc		_((int));
extern	bool	breakflow	_((INST *));
extern	bool	ubreakflow	_((INST *));

/*
 * inst.c
 */
extern	INST	*addinst	_((INST *,char *));     /* parse line */
extern	int	stomask 	_((char *));
extern	bool	putop		_((char *,struct opnd *));

/*
 * instbis.c
 */
extern	void	delinst 	_((INST *));
extern	INST	*insinst	_((INST *));            /* add after */
extern	INST	*inspreinst	_((INST *));            /* add before */
extern	INST	*setupinst	_((INST *, ...));
extern	bool	opeq		_((struct opnd *,struct opnd *));
extern	void	putinst 	_((FILE *,INST *));
extern	char	*masktos	_((int));

/*
 * io.c
 */
extern	char	*readline	_((FILE *));
extern	void	tokenize	_((char *,int *,\
					char **,char **,char **,char **));

/*
 * source.c
 */
extern	void	putoptions	_((FILE *));
extern	INST	*getsource	_((char *));
extern	void	putsource	_((char *));

/*
 * util.c
 */
extern	FILE	*safeopen	_((char *,char *));
extern	void	safeclose	_((FILE *));
extern	char	*alloc		_((int));
extern	char	*strsave	_((char *));

/*
 * sym.c
 */
extern	void	freeop		_((struct opnd *));
extern	void	freesym 	_(());
extern	char	*mktmp		_(());

/*
 * dumpline.c
 */
extern	void	dumpline	_((INST *));

/*
 *	regmask.c
 */
extern	void	fix_movem	_(());
extern	void	fix_link	_(());

/*
 *	peep1.c
 */
extern	void	peep		_(());
extern	int	nb_bits 	_((unsigned int));
extern	int	first_nb	_((unsigned int));
extern	void	fix_pea 	_(());

/*
 *	peep2.c
 */
extern	bool	peep2		_(());

/*
 *	peep2bis.c
 */
extern	INST	*ipeep2bis	_((INST *));

/*
 *	peep2ter.c
 */
extern	INST	*ipeep2ter	_((INST *));

/*
 *	peep2_4.c
 */
extern	INST	*ipeep2_4	_((INST *));

/*
 *	peep2_5.c
 */
extern	INST	*ipeep2_5	_((INST *));

/*
 *	peep3.c
 */
extern	bool	peep3		_(());

/*
 *	peep3bis.c
 */
extern	INST	*ipeep3bis	_((INST *));

/*
 *	peep4.c
 */
extern	bool	peep4		_(());

/*
 *	ulink.c
 */
extern	void	ulink		_(());

/*
 *	asp68k.c
 */
extern	bool	asp68k		_(());

/*
 *	asp68kbis.c
 */
extern	INST	*peepabis	_(());

/*
 *	mulopt.c
 */
extern	bool	mulopt		_((INST *));

/*
 *	labmerge.c
 */
extern	void	labmerge	_(());

/*
 *	jsropt.c
 */
extern	void	peepjsr 	_(());

/*
 *	mc60opt.c
 */
extern	bool	mc50opt 	_(());

/*
 *	mc40opt.c
 */
extern	bool	mc40opt 	_(());
extern	int	do_arithm	_((int,int,int));

/*
 *	mc20opt.c
 */
extern	bool	mc20opt 	_(());

/*
 *	stackopt.c
 */
extern	bool	stackopt 	_(());

/*
 *	movem.c
 */
extern	void	movem		_(());

/*
 *	progress.c
 */
extern	void	progress	_(());
