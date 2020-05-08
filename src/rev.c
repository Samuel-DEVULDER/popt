#define cctimes 678
#define VNAME "POPT"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef VNUM
#define VNUM "0.0"
#endif

#ifdef __PDC__
char IDstring[1]={0};
static char IDstring1[]="$VER: POPT "VNUM" (08.10.97) [c: 678] © by Samuel Devulder.";
#else
char IDstring[]="\0$VER: POPT "VNUM" (08.10.97) [c: 678] © by Samuel Devulder.";
#endif

static void _revinfo(char *name)
	{
	printf("%s\n\n",((char*)IDstring)+1);
	printf("revinfo(\"%s\"): 678 compilations\n",name);
	puts("    asp68k.o\t\t\t\t    4639  Wed Oct  8 22:40:50 1997");
	puts("    asp68kbis.o\t\t\t\t    2471  Wed Oct  8 22:41:17 1997");
	puts("    branchopt.o\t\t\t\t    5483  Wed Oct  8 22:43:05 1997");
	puts("    branchopt2.o\t\t\t    3082  Wed Oct  8 23:19:24 1997");
	puts("    data.o\t\t\t\t    3907  Wed Oct  8 22:34:33 1997");
	puts("    dumpline.o\t\t\t\t     769  Wed Oct  8 22:33:23 1997");
	puts("    health.o\t\t\t\t    3759  Wed Oct  8 22:33:58 1997");
	puts("    inst.o\t\t\t\t    7145  Wed Oct  8 22:31:01 1997");
	puts("    instbis.o\t\t\t\t    4214  Wed Oct  8 22:31:34 1997");
	puts("    io.o\t\t\t\t    1019  Wed Oct  8 22:31:54 1997");
	puts("    jsropt.o\t\t\t\t    1298  Wed Oct  8 22:45:31 1997");
	puts("    labmerge.o\t\t\t\t    1591  Wed Oct  8 22:42:27 1997");
	puts("    main.o\t\t\t\t    6449  Wed Oct  8 22:32:22 1997");
	puts("    mc20opt.o\t\t\t\t    5139  Wed Oct  8 22:45:10 1997");
	puts("    mc40opt.o\t\t\t\t    6448  Wed Oct  8 22:44:28 1997");
	puts("    movem.o\t\t\t\t    4428  Wed Oct  8 22:46:07 1997");
	puts("    mulopt.o\t\t\t\t    4757  Wed Oct  8 22:42:05 1997");
	puts("    opcodes.o\t\t\t\t    2906  Wed Oct  8 22:32:31 1997");
	puts("    peep1.o\t\t\t\t    6095  Wed Oct  8 22:47:35 1997");
	puts("    peep2.o\t\t\t\t    3883  Wed Oct  8 22:36:01 1997");
	puts("    peep2_2.o\t\t\t\t    4192  Wed Oct  8 22:48:10 1997");
	puts("    peep2_3.o\t\t\t\t    3410  Wed Oct  8 22:37:10 1997");
	puts("    peep2_4.o\t\t\t\t    3769  Wed Oct  8 22:37:42 1997");
	puts("    peep2_5.o\t\t\t\t    4077  Wed Oct  8 22:38:18 1997");
	puts("    peep3.o\t\t\t\t    3433  Wed Oct  8 22:48:39 1997");
	puts("    peep3bis.o\t\t\t\t    3760  Wed Oct  8 22:39:24 1997");
	puts("    peep4.o\t\t\t\t    2424  Wed Oct  8 22:39:51 1997");
	puts("    progress.o\t\t\t\t     473  Wed Oct  8 22:46:23 1997");
	puts("    regmask.o\t\t\t\t    2164  Wed Oct  8 22:34:54 1997");
	puts("    source.o\t\t\t\t    1948  Wed Oct  8 22:30:17 1997");
	puts("    stackopt.o\t\t\t\t     847  Wed Oct  8 22:46:42 1997");
	puts("    sym.o\t\t\t\t     587  Wed Oct  8 22:32:49 1997");
	puts("    ulink.o\t\t\t\t     956  Wed Oct  8 22:40:14 1997");
	puts("    util.o\t\t\t\t     780  Wed Oct  8 22:33:06 1997");
	}

void revinfo(int argc, char **argv)
	{
	register int i;
	for(i=1;i<argc;++i)
		if(!strcmp(argv[i],"-revinfo"))
			{_revinfo(argv[0]);exit(0);}
	}
