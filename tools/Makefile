TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

mkfs: mkfs.c
	gcc -Werror -Wall -g -o mkfs mkfs.c

clean:
	rm -f _* mkfs
