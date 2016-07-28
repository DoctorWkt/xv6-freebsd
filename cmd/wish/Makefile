# The Makefile for Wish 2.0 
#
# $Revision: 1.1 $ $Date: 2016/07/22 12:03:23 $
#
# Insert compiler flags here.
# CFLAGS are used when compiling each file
# LDFLAGS are used when linking
#
TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

#CFLAGS=-m
#CFLAGS=-g
#CFLAGS=-g -Wall -traditional -v
#CFLAGS= -O
#CFLAGS= -Wall
#CFLAGS= -pg

#LDFLAGS= -3
#LDFLAGS= -i
#LDFLAGS= -p
#LDFLAGS= -pg
LDFLAGS += -N -Ttext 0

# And the name of your compiler (may not be cc)
#CC=/usr/bin2/cc
#CC=/local/bin/bcc
#CC=cc

# Some compilers generate object files that don't end in `.o'. Alter the
# following variable if this is the case
#O= s
O= o

# Special defines for the compilation stage.
#
#DEFINES=-ZP			# A/UX 2.01

# Libraries to include during the link stage. On some machines (like OSx under
# the ucb universe, you may need to get some ANSI C string functions that are
# not in libc.a. These are: strtok.o strpbrk.o strspn.o strcspn.o
#
#CLIB=-lcurses			# OSX5_1_ATT GENSYSV
#CLIB=-ltermcap			# OSX5_1_BSD GENBSD ULTRIX J386BSD0_1
#				  SUNOS3 SUNOS4	FREEBSD_1 FREEBSD_2
#CLIB=-ltermcap strtok.o strpbrk.o strspn.o strcspn.o	# OSX5_1_BSD maybe
#CLIB=-ltermcap -lposix		# AUX2_01
#CLIB= -lterm -lndir		# COHERENT
CLIB=../../lib/libc.a ../../lib/libtermcap.a $(LIBGCC)

# On some systems (PC Minix, Coherent), we need to set the final stack size.
#STACK= 30000
#FINAL_TOUCHES= fixstack $(STACK) wish ; chmod 755 wish ; size wish # COHERENT
#FINAL_TOUCHES= chmem '=$(STACK)' wish	# MINIX1_5 MINIX1_7
FINAL_TOUCHES= @echo 'All done'


# ====== END OF CONFIGURABLE SECTION ======



SRCS=   alias.c bind.c builtin.c clebuf.c clex.c comlined.c exec.c file.c \
	hist.c job.c main.c malloc.c meta.c parse.c prints.c \
	signal.c term.c val.c var.c

PSRCS=  alias.c bind.c builtin.c clebuf.c clex.c comlined.c exec.c file.c \
	hist.c job.c posixjob.c main.c malloc.c meta.c parse.c \
	signal.c term.c val.c var.c 

OBJS=   alias.$(O) bind.$(O) builtin.$(O) clebuf.$(O) clex.$(O) comlined.$(O) \
	exec.$(O) file.$(O) hist.$(O) job.$(O) main.$(O) malloc.$(O) \
	meta.$(O) parse.$(O) prints.$(O) signal.$(O) term.$(O) val.$(O) \
	var.$(O)

all: wish

wish : $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o wish $(CLIB)
	$(FINAL_TOUCHES)
#	strip wish

install: wish
	cp wish ../../fs/bin

$(OBJS): Makefile header.h machine.h

job.c:	posixjob.c ucbjob.c v7job.c

# For Coherent, comment the next two lines out!
.c.$(O): $<
	$(CC) $(CFLAGS) $(DEFINES) -c $<

lint : $(SRCS)
	lint $(LINTFLAGS) $(DEFINES) $(SRCS)

proto:	$(PSRCS)
	proto $(PSRCS) | tail +7 | egrep -v 'undef|getaline' > Proto/p2
	cat Proto/p1 Proto/p2 Proto/p3 > proto.h

clean :
	rm -f *.$(O) wish

clr :
	rm -f *.$(O) wish

realclean:
	rm -f *.$(O) wish
