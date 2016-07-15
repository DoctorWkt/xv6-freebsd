TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
CFLAGS = -fno-builtin -Wall -m32 -Werror -nostdinc -I$(INCLDIR)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

UPROGS= _basename _cal _cksum _cmp _comm _crc \
	_cut _expand _newcat _newwc _ttt _uue \
	_isatty-test

all: ../../lib/libc.a $(UPROGS)
	mv _* ../..

../../lib/libc.a:
	$(MAKE) -C ../../lib libc.a

_%: %.o ../../lib/libc.a
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

clean:
	rm -f _* *.asm *.o *.sym *.d
