#	@(#)Makefile	4.7 (Berkeley) 5/11/90

TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
#CFLAGS=-g

# Cancel the .c -> output implicit rule
%: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

CFLAGS+=-DNROFF -DVMUNIX
SRCS=	ni.c nii.c n1.c n2.c n3.c n4.c n5.c n6.c n7.c n8.c n9.c n10.c \
	hytab.c ntab.c suftab.c
OBJS=	ni.o nii.o n1.o n2.o n3.o n4.o n5.o n6.o n7.o n8.o n9.o n10.o \
	hytab.o ntab.o suftab.o

all: nroff

nroff: $(OBJS) ../../lib/libc.a $(LIBGCC)
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^

fred: $(OBJS)
	cc -o fred $(CFLAGS) $(OBJS)

install: nroff
	strip nroff
	cp nroff ../../fs/bin

clean:
	rm -f nroff $(OBJS) fred
