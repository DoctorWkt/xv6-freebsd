# Makefile for make (!)
TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

CFLAGS += -Dunix -D_MINIX -D_POSIX_SOURCE

OBJ =	check.o input.o macro.o main.o make.o reader.o rules.o archive.o

all:	make

make: $(OBJ) ../../lib/libc.a
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^
	strip $@

install: make
	cp make ../../fs/bin

$(OBJ): h.h

clean:
	rm -f *.o *.bak core make
