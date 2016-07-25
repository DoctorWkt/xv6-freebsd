TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

all: bcmp.o bcopy.o bzero.o ffs.o index.o memccpy.o memchr.o memcmp.o \
	memset.o rindex.o strcasecmp.o strcat.o strcmp.o strcoll.o \
	strcpy.o strcspn.o strdup.o strftime.o strlen.o strmode.o \
	strncat.o strncmp.o strncpy.o strpbrk.o strsep.o strspn.o \
	strstr.o strtok.o strxfrm.o swab.o memmove.o strchr.o strrchr.o \
	memcpy.o

memmove.o: bcopy.c
	$(CC) $(CFLAGS) -DMEMMOVE -c -o memmove.o bcopy.c

memcpy.o: bcopy.c
	$(CC) $(CFLAGS) -DMEMCOPY -c -o memcpy.o bcopy.c

strchr.o: index.c
	$(CC) $(CFLAGS) -DSTRCHR -c -o strchr.o index.c

strrchr.o: rindex.c
	$(CC) $(CFLAGS) -DSTRRCHR -c -o strrchr.o rindex.c

clean:
	rm -f *.o
