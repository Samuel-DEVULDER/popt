#
#	Makefile for popt
#
#	(c) by Samuel DEVULDER
#

VERSION = 1.5

MAINDIR = ../
WORKDIR = $(MAINDIR)src/
POPT	= $(MAINDIR)popt
TST	= $(MAINDIR)test
DHR	= $(MAINDIR)dhr
BIDON	= $(MAINDIR)bidon
PEEPDOC = $(MAINDIR)peephole.doc
POPTREAD= $(MAINDIR)popt.readme
POPTDOC = $(MAINDIR)popt.doc

all: $(POPT) $(PEEPDOC) $(DHR)

apurify:
	make APURIFY=1

clean:
	delete >nil: #?.o

CCFLAGS = -O3 -m68030 -fomit-frame-pointer -noixemul
LNFLAGS = -noixemul
CC	= gcc $(CCFLAGS)
LN	= gcc $(LNFLAGS)

FULLARCHIVE	= ram:fullpopt.lha
ARCHIVE 	= ram:popt.lha
LHA		= lha

COMPOS		= source inst instbis io main opcodes sym util dumpline \
health data regmask peep1 peep2 peep2_2 peep2_3 peep2_4 peep2_5 peep3 \
peep3bis peep4 ulink asp68k asp68kbis mulopt labmerge branchopt branchopt2 \
mc60opt mc40opt mc20opt jsropt movem progress stackopt

OPTSRC		= peep1.c peep2.c peep2_2.c peep2_3.c peep2_4.c peep2_5.c \
peep3.c peep3bis.c peep4.c asp68k.c asp68kbis.c mc60opt.c mc40opt.c \
mc20opt.c ulink.c branchopt.c branchopt2.c jsropt.c movem.c stackopt.c

OBJS		= $(addsuffix .o,$(COMPOS))

LINKF		= linkfile

fullarchive:
	cd $(MAINDIR)/
	$(LHA) -x -r u $(FULLARCHIVE) popt/~(#?.o)
	cd $(MAINDIR)src

archive:
	cd $(MAINDIR)/
	$(LHA) -x u $(ARCHIVE) popt/popt popt/popt.apurify\
	popt/popt.readme popt/popt.doc popt/peephole.doc popt/dhr.a popt/dhr0?0.a popt/dhr0?0i.a
	cd $(MAINDIR)src

$(POPT): $(OBJS) rev.o
	$(LN) -o $(POPT) $(OBJS) rev.o

rev.o: $(OBJS) mkrev
	mkrev rev.c @$(LINKF)
	$(CC) -c rev.c -o $@ -DVNUM='"$(VERSION)"'

.c.o: popt.h
	$(CC) -c $< -o $@

#
# makerev : revision number management
#
mkrev: mkrev.c
	$(CC) $< -o $@

#
# makedoc : documentation management
#
mkdoc: mkdoc.c
	$(CC) $< -o $@

$(PEEPDOC): mkdoc $(OPTSRC)
	mkdoc >$@ $(OPTSRC)

#
# dhr
#
$(DHR): $(DHR).c $(POPT)
#	cc +l -a $(DHR).c -o $(DHR).a
#	as $(DHR).a -o $(DHR).o
#	ln $(DHR).o -lc32 -o $(DHR)
#	$(POPT) -vd $(DHR).a -o $(DHR)000.a
#	as $(DHR)000.a -o $(DHR)000.o
#	ln $(DHR)000.o -lc32 -o $(DHR)000
#	$(POPT) -vd2 $(DHR).a -o $(DHR)020.a
#	as $(DHR)020.a -o $(DHR)020.o
#	ln $(DHR)020.o -lc32 -o $(DHR)020
#	$(POPT) -vd4 $(DHR).a -o $(DHR)040.a
#	as $(DHR)040.a -o $(DHR)040.o
#	ln $(DHR)040.o -lc32 -o $(DHR)040
#	$(POPT) -vd2ni $(DHR).a -o $(DHR)020i.a
#	as $(DHR)020i.a -o $(DHR)020i.o
#	ln $(DHR)020i.o -lc32 -o $(DHR)020i
#	$(POPT) -vd4ni $(DHR).a -o $(DHR)040i.a
#	as $(DHR)040i.a -o $(DHR)040i.o
#	ln $(DHR)040i.o -lc32 -o $(DHR)040i
#	-$(DHR)
#	-$(DHR)000
#	-$(DHR)020
#	-$(DHR)040
#	-$(DHR)020i
#	-$(DHR)040i
	dcc -s -a $(DHR).c -o $(DHR).a
	dcc -s -s $(DHR).a -o $(DHR)
	$(POPT) -vd $(DHR).a -o $(DHR)000.a
	dcc -s -s $(DHR)000.a -o $(DHR)000
	$(POPT) -vd2 $(DHR).a -o $(DHR)020.a
	dcc -s -s $(DHR)020.a -o $(DHR)020
	$(POPT) -vd2ni $(DHR).a -o $(DHR)020i.a
	dcc -s -s $(DHR)020i.a -o $(DHR)020i
	$(POPT) -vd4 $(DHR).a -o $(DHR)040.a
	dcc -s -s $(DHR)040.a -o $(DHR)040
	$(POPT) -vd4ni $(DHR).a -o $(DHR)040i.a
	dcc -s -s $(DHR)040i.a -o $(DHR)040i
	$(LHA) u /benchpack.lha $(DHR)#?
	-$(DHR)
	-$(DHR)000
	-$(DHR)020
	-$(DHR)040
	-$(DHR)020i
	-$(DHR)040i

#
# test
#
$(TST).s: $*.c
	dcc -s -a $*.c -o $*.s

$(TST).a: $(TST).s $(POPT)
	$(POPT) -dv $(TST).s -o $(TST).a
	$(POPT) -dv2 $(TST).s -o $(TST)2.a
	$(POPT) -dv4 $(TST).s -o $(TST)4.a

#
# bidon
#
$(BIDON).s: $*.c
	dcc -s -a $*.c -o $*.s

$(BIDON).a: $(BIDON).s $(POPT)
	$(POPT) -dv $(BIDON).s -o $(BIDON).a
	$(POPT) -dv4 $(BIDON).s -o $(BIDON)4.a
