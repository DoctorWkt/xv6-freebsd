TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

# I've removed -Wall from the CFLAGS:	Warren
CFLAGS = -fno-builtin -m32 -Werror -nostdinc -I$(INCLDIR)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
CFLAGS+= -D_POSIX_SOURCE

all: regcomp.o regerror.o regexec.o regfree.o

clean:
	rm -f *.o
