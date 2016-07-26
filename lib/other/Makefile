TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
CFLAGS+= -D_POSIX_SOURCE

all: getopt.o v8regerror.o v8regexp.o v8regsub.o getcwd.o mktemp.o dup2.o \
	fdopen.o termcap.o

clean:
	rm -f *.o
