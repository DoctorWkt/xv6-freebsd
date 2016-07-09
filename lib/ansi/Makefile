TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

INCLDIR=$(TOP_SRCDIR)/include
CFLAGS = -fno-builtin -Wall -m32 -Werror -nostdinc -I$(INCLDIR)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

all: abort.o abs.o asctime.o assert.o atexit.o atof.o atoi.o atol.o \
	bsearch.o calloc.o chartab.o clock.o ctime.o difftime.o div.o \
	errlist.o getenv.o gmtime.o isalnum.o isalpha.o \
	isascii.o iscntrl.o isdigit.o isgraph.o islower.o isprint.o \
	ispunct.o isspace.o isupper.o isxdigit.o labs.o ldiv.o localeconv.o \
	localtime.o malloc.o mblen.o mbstowcs.o mbtowc.o memchr.o memcmp.o \
	memcpy.o memmove.o memset.o misc.o mktime.o qsort.o raise.o rand.o \
	setlocale.o strcat.o strchr.o strcmp.o strcoll.o \
	strcpy.o strcspn.o strerror.o strftime.o strlen.o strncat.o \
	strncmp.o strncpy.o strpbrk.o strrchr.o strspn.o strstr.o \
	strtok.o strtol.o strxfrm.o system.o tolower.o toupper.o tzset.o \
	wcstombs.o wctomb.o

clean:
	rm -f *.o
