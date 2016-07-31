#	from: @(#)Makefile	8.1 (Berkeley) 6/6/93
TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

UPROGS= sed
OBJS=	compile.o main.o misc.o process.o


all: ../../lib/libc.a ../../fs/bin $(UPROGS)

install: all
	strip $(UPROGS)
	cp $(UPROGS) ../../fs/bin

# Cancel the .c -> output implicit rule
%: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(UPROGS): $(OBJS) ../../lib/libc.a $(LIBGCC)
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^

clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
