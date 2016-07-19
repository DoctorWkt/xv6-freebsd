TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

INCLDIR=$(TOP_SRCDIR)/include
CFLAGS = -fno-builtin -Wall -m32 -Werror -nostdinc -I$(INCLDIR) -D_POSIX_SOURCE
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

all: getopt.o v8regerror.o v8regexp.o v8regsub.o getcwd.o mktemp.o

clean:
	rm -f *.o
