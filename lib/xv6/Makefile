TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

INCLDIR=$(TOP_SRCDIR)/include
CFLAGS = -fno-builtin -Wall -m32 -Werror -nostdinc -I$(INCLDIR) -D_POSIX_SOURCE
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

all: chown.o chmod.o creat.o stat.o getuid.o umask.o mkfifo.o \
        time.o utime.o rename.o rmdir.o usys.o oldmalloc.o oldprintf.o \
	tcgetattr.o opendir.o fstat.o access.o sleep.o

clean:
	rm -f *.o
