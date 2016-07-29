TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

UPROGS= cat ls date sleep rmdir rm mv ln hostname cal \
	basename cmp comm cut dirname expand bdes yes \
	false true du head join printenv rev split \
	strings tee uniq wc echo xargs yes egrep grep \
	fgrep ed man

all: ../../lib/libc.a ../../fs/bin $(UPROGS)

grep: egrep
	cp egrep grep

fgrep: egrep
	cp egrep fgrep

install: all
	cp $(UPROGS) ../../fs/bin

../../fs/bin:
	mkdir -p ../../fs/bin

../../lib/libc.a:
	$(MAKE) -C ../../lib libc.a

# Cancel the .c -> output implicit rule
%: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%: %.o ../../lib/libc.a $(LIBGCC)
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^
#	strip $@

clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
