TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

mkfs: mkfs.c
#	gcc -Werror -Wall -I../include -Wno-builtin-declaration-mismatch -g -o mkfs mkfs.c
	gcc -Werror -Wall -idirafter ../include -g -o mkfs mkfs.c

clean:
	rm -f _* mkfs
