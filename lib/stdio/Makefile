TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
CFLAGS+= -D_POSIX_SOURCE -DNOFLOAT

all: clearerr.o data.o doprnt.o doscan.o ecvt.o fclose.o feof.o ferror.o \
	fillbuf.o fflush.o fgetc.o fgetpos.o fgets.o fileno.o flushbuf.o \
	fopen.o fprintf.o fputc.o fputs.o fread.o freopen.o fscanf.o fseek.o \
	fsetpos.o ftell.o fwrite.o getc.o getchar.o gets.o icompute.o \
	perror.o printf.o putc.o putchar.o puts.o remove.o rewind.o \
	scanf.o setbuf.o setvbuf.o sprintf.o sscanf.o tmpfile.o tmpnam.o \
	ungetc.o vfprintf.o vprintf.o vscanf.o vsprintf.o vsscanf.o

clean:
	rm -f *.o
