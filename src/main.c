/*
 *	main.c - analyse cmd line & start processing
 *
 *	(c) by Samuel DEVULDER
 */
#include "popt.h"

/*
 * Options
 */
#ifdef DEBUG
bool	debug	     = FALSE;
#endif

bool	safe	     = FALSE;
bool	verbose      = FALSE;
bool	size	     = FALSE;
bool    mc60         = FALSE;
bool	mc40	     = FALSE;
bool	mc20	     = FALSE;
bool	newsn	     = FALSE;
bool	keeplbl      = FALSE;
bool	keepcomment  = FALSE;
bool	slflag	     = FALSE;
bool	nowarning    = FALSE;
bool	nostack      = FALSE;
bool	newinsts     = FALSE;
bool	last_pass    = FALSE;
int	cr_regs      = 0/*RM(D0)|RM(D1)|RM(A0)|RM(A1)*/;
int	cs_regs      = RM(D0)|RM(D1)|RM(A0)|RM(A1) /* these are sratch-regs*/;
int	used_rts     = RM(D0)|RM(D2)|RM(D3)|RM(D4)|RM(D5)|RM(D6)|RM(D7)|
			      RM(A2)|RM(A3)|RM(A4)|RM(A5)|RM(A6);

/*
 * Optimization statistics (use -v to print)
 */
int	s_bdel	   = 0; 	/* branches deleted */
int	s_brev	   = 0; 	/* branches reversed */
int	s_link	   = 0; 	/* link/unlink deleted */
int	s_labmerge = 0; 	/* label merged */
int	s_peep1    = 0; 	/* 1 instruction peephole changes */
int	s_peep2    = 0; 	/* 2 instruction peephole changes */
int	s_peep3    = 0; 	/* 3 instruction peephole changes */
int	s_peep4    = 0; 	/* 4 instruction peephole changes */
int	s_asp68k   = 0; 	/* asp68k project optimizations */
int	s_idel	   = 0; 	/* instructions deleted */
int	s_jsr	   = 0; 	/* jsr/bsr modified */
int	s_mc60	   = 0; 	/* 68060 optimizations */
int	s_mc40	   = 0; 	/* 68040 optimizations */
int	s_mc20	   = 0; 	/* 68020 optimizations */
int	s_movem    = 0; 	/* movem strip */

char	*ProgName;		/* name of this program */

void usage()
	{
	printf(
"\nUsage: %s [-revinfo] [flags] <inputfile.asm> [-o <outputfile.asm>]\n\n",
		ProgName);
	printf("Valid optimizer flags are:\n");
#ifdef DEBUG
	printf("   -d            : Debug\n");
#endif
	printf("   -v            : Verbose\n");
	printf("   -s            : Safe optimization only\n");
	printf("   -z            : Optimize size\n");
	printf("   -cr <REGMASK> : Set regs refs in jsr (def=%s)\n",cr_regs?masktos(cr_regs):"''");
	printf("   -cs <REGMASK> : Set regs sets in jsr (def=%s)\n",cs_regs?masktos(cs_regs):"''");
	printf("   -ru <REGMASK> : Set regs used after rts (def=%s)\n",used_rts?masktos(used_rts):"''");
	printf("   -6            : Optimize for 68060\n");	
	printf("   -4            : Optimize for 68040\n");
	printf("   -2            : Optimize for 68020/030\n");
	printf("   -ns           : New syntax output\n");
	printf("   -nw           : Do not display warnings\n");
	printf("   -np           : Suppress stack fixup\n");
	printf("   -ni           : Allow usage of new instructions for 68020+\n");
	printf("   -kc           : Keep lines only containing comments\n");
	printf("   -kl           : Keep unused label\n");
	printf("   -sl           : Put labels in separate lines\n");
	printf("   -h            : This usage (same as -? or ?)\n");
	printf("\n%s\n\n",IDstring+7);
	exit(0);
	}

#ifdef	APURIFY
extern void AP_Init(),AP_Close();
#endif

main(argc,argv)
	int	argc;
	char	*argv[];
	{
	char	*source_name;
	char	*dest_name;
	int	i;
	INST	*source;

#ifdef	APURIFY
	AP_Init(); atexit(AP_Close); AP_Report(0);
#endif
	revinfo(argc,argv);
	ProgName = argv[0];
	source_name = dest_name = NULL;
	/*
	 *	parse command line
	 */
	for(i=1;i<argc;++i)
		{
		char *t=argv[i];

		if(*t=='?' && !t[1]) usage();
		else if(*t=='-')
			{
			ifn(t[1])
				{
				printf("%s: Missing flag !\n",ProgName);
				usage();
				}
			else while(*++t)
				{
switch(*t)
	{
#ifdef DEBUG
	case 'd': case 'D':
	debug	= TRUE;
	break;
#endif
	case 'h': case 'H': case '?':
	usage();

	case 'v': case 'V':
	verbose = TRUE;
	break;

	case 'z': case 'Z':
	size	= TRUE;
	break;

	case '6':
	mc60	= TRUE;
	break;

	case '4':
	mc40	= TRUE;
	break;

	case '2':
	mc20	= TRUE;
	break;

	case 'n': case 'N':
	++t;
	if(tolower(*t)=='s') newsn=TRUE;
	else if(tolower(*t)=='w') nowarning=TRUE;
	else if(tolower(*t)=='p') nostack=TRUE;
	else if(tolower(*t)=='i') newinsts=TRUE;
	else	goto err2;
	break;

	case 'k': case 'K':
	++t;
	if(tolower(*t)=='c') keepcomment=TRUE;
	else if(tolower(*t)=='l') keeplbl=TRUE;
	else goto err2;
	break;

	case 'o': case 'O':
	if(dest_name)
		{
		printf("%s: dest file already set to \"%s\".\n",
			ProgName,dest_name);
		usage();
		}
	if(t[1]){dest_name = ++t;while(t[1]) ++t;}
	else	{
		if(++i<argc) dest_name = argv[i];
		else	{
			printf("%s: Missing filename after -o !\n",ProgName);
			usage();
			}
		}
	break;

	case 'r': case 'R':
	++t;
	if(tolower(*t)!='u')
		{
		printf("%s: Unknown option: %c%c\n",ProgName, t[-1],t[0]);
		usage();
		}
	if(t[1]){used_rts=stomask(t+1);while(t[1]) ++t;}
	else	{
		if(++i<argc) used_rts=stomask(argv[i]);
		else	{
			printf("%s: Missing mask of registers after -ru !\n",
				ProgName);
			usage();
			}
		}
	break;

	case 'c': case 'C':
	++t;
	if(tolower(*t)!='r' && tolower(*t)!='s')
		{
err2:		printf("%s: Unknown option: %c%c\n",ProgName, t[-1],t[0]);
		usage();
		}
	if(t[1]){
		if(tolower(*t)=='r')    cr_regs=stomask(t+1);
		else			cs_regs=stomask(t+1);
		while(t[1]) ++t;
		}
	else	{
		if(++i<argc)
			{
			if(tolower(*t)=='r')    cr_regs=stomask(argv[i]);
			else			cs_regs=stomask(argv[i]);
			}
		else	{
			printf("%s: Missing mask of registers after -c !\n",
				ProgName);
			usage();
			}
		}
	break;

	case 's': case 'S':
	if(tolower(t[1])=='l') {slflag=TRUE;++t;}
	else safe=TRUE;
	break;

	default:
	printf("%s: Unknown option: %c\n",ProgName, *t);
	usage();
	}
				}
			}
		else if(source_name)
			{
			printf("%s: source file already set to \"%s\". What is \"%s\" ?\n",
				ProgName,source_name,argv[i]);
			usage();
			}
		else	source_name = argv[i];
		}

#ifdef	APURIFY
	AP_Report(1);
#endif

	ifn(source_name)
		{
		printf("%s: no source file !\n",ProgName);
		usage();
		}

	ifn(dest_name) dest_name = source_name;

	if(verbose) printf("Processing \"%s\"->\"%s\": ",source_name,dest_name);
	if(verbose) putoptions(stdout);
	if(verbose && debug)
		{
		printf("call_refs=%s ",cr_regs?masktos(cr_regs):"''");
		printf("call_sets=%s ",cs_regs?masktos(cs_regs):"''");
		printf("used_rts=%s ",used_rts?masktos(used_rts):"''");
		}
	if(verbose) printf("\n");

	getsource(source_name);

	fix_movem();/*  putsource("*"); /**/
	fix_link();/*   putsource("*"); /**/
	ulink();/*      putsource("ram:t1");    /**/
	labmerge();/*   putsource("ram:t2");    /**/
	branchopt();/*  putsource("ram:t3");    /**/
	peepjsr();/*    putsource("ram:t4");    /**/
	do_health();/*  putsource("*"); /**/
	peep();         /**/
	/* try to invert */
	try_swap();
	movem();
/*	labmerge();     /* done in do_health() */
	do_health();
	last_pass = TRUE;
	peep();

	fix_pea();

	putsource(dest_name);

	freesym();

	if(verbose)
		{
		printf("  Peephole changes (1)   : %4d\n", s_peep1);
		printf("  Peephole changes (2)   : %4d\n", s_peep2);
		printf("  Peephole changes (3)   : %4d\n", s_peep3);
		printf("  Peephole changes (4)   : %4d\n", s_peep4);
		printf("  Asp68k changes         : %4d\n", s_asp68k);
		if(mc60)
		printf("  68060 changes          : %4d\n", s_mc60);
		if(mc40)
		printf("  68040 changes          : %4d\n", s_mc40);
		if(mc20)
		printf("  68020 changes          : %4d\n", s_mc20);
		printf("  Instructions deleted   : %4d\n", s_idel);
		printf("  Branches removed       : %4d\n", s_bdel);
		printf("  Branches reversed      : %4d\n", s_brev);
		printf("  Link/Unlink removed    : %4d\n", s_link);
		printf("  Labels merged          : %4d\n", s_labmerge);
		printf("  Modified jmp/jsr       : %4d\n", s_jsr);
		printf("  Stripped movem         : %4d\n", s_movem);
		ifn(nostack)
		printf("  Stack fixup            : %4d\n", s_stack);
		}
	exit(0);
	}
