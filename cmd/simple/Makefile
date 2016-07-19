TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
CFLAGS = -fno-builtin -Wall -m32 -Werror -nostdinc -I$(INCLDIR)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

UPROGS= basename cal cksum cmp comm cp crc \
	cut ed ls cat wc ttt uue mv rm \
	showtime grep date head tail od yes pwd \
	uud uniq tr tsort expand unexpand split \
	strings tee paste join

all: ../../lib/libc.a ../../fs/bin $(UPROGS)
	mv $(UPROGS) ../../fs/bin

../../fs/bin:
	mkdir -p ../../fs/bin

../../lib/libc.a:
	$(MAKE) -C ../../lib libc.a

# Cancel the .c -> output implicit rule
%: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%: %.o ../../lib/libc.a
#	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^
	strip $@

clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
