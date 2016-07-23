# Makefile for mined

TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
CFLAGS	+= -D_MINIX -D_POSIX_SOURCE

OBJ = mined1.o mined2.o

all:	mined

mined:	$(OBJ)
	$(LD) $(LDFLAGS) -N -Ttext 0 -o mined $(OBJ) ../../lib/libc.a
	strip mined

install: mined
	cp mined ../../fs/bin

$(OBJ):	mined.h

clean:
	rm -f mined *.o *.s core *.bak
