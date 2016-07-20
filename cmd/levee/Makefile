TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
CFLAGS = -fno-builtin -Wall -m32 -Werror -nostdinc -I$(INCLDIR) -D_POSIX_SOURCE
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# makefile for Levee

OBJS = blockio.o display.o editcor.o exec.o find.o \
    unixcall.o globals.o insert.o main.o misc.o \
    modify.o move.o ucsd.o undo.o wildargs.o \
    version.o

all: lev

lev: $(OBJS) ../../lib/libc.a
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^
	strip lev
	cp lev ../../fs/bin

version.o: version.c VERSION
	$(CC) $(CFLAGS) -c -DVERSION=\"`cat VERSION`\" version.c

clean:
	rm -f *.o lev

# Dependencies
blockio.o    : levee.h extern.h blockio.c config.h
display.o    : levee.h extern.h termcap.i display.c config.h
editcor.o    : levee.h extern.h editcor.c config.h
exec.o       : levee.h extern.h exec.c config.h
find.o       : levee.h extern.h grep.h find.c config.h
globals.o    : levee.h globals.c config.h
insert.o     : levee.h extern.h insert.c config.h
main.o       : levee.h extern.h main.c config.h
misc.o       : levee.h extern.h misc.c config.h
modify.o     : levee.h extern.h grep.h modify.c config.h
move.o       : levee.h extern.h move.c config.h
rmxcall.o    : levee.h rmxcall.c config.h
ucsd.o       : levee.h extern.h ucsd.c config.h
undo.o       : levee.h extern.h undo.c config.h
unixcall.o   : levee.h extern.h unixcall.c config.h
wildargs.o   : levee.h extern.h wildargs.c config.h
