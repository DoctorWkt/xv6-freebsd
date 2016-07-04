TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

mkfs: mkfs.c
	gcc -Werror -Wall -o mkfs mkfs.c

clean:
	rm -f _* mkfs
