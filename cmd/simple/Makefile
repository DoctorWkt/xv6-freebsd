TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

UPROGS= basename cal cksum cmp comm cp crc \
	cut ed ls cat wc ttt uue mv rm \
	showtime grep date head tail od yes pwd \
	uud uniq tr tsort expand unexpand split \
	strings tee paste join env tryraw roff tryexit

all: ../../lib/libc.a ../../fs/bin $(UPROGS)

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

%: %.o ../../lib/libc.a
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^
	strip $@

clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
