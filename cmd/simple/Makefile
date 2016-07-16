TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
CFLAGS = -fno-builtin -Wall -m32 -Werror -nostdinc -I$(INCLDIR)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

UPROGS= basename cal cksum cmp comm cp crc \
	cut expand newcat newwc ttt uue \
	isatty-test odir-test open-test

all: ../../lib/libc.a $(UPROGS)
	mv $(UPROGS) ../../fs

../../lib/libc.a:
	$(MAKE) -C ../../lib libc.a

# Cancel the .c -> output implicit rule
%: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%: %.o ../../lib/libc.a
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
