TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

UPROGS= cat ls tryreadwrite date recurse tryraw sleep rmdir rm

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

%: %.o ../../lib/libc.a /usr/lib/gcc/i686-linux-gnu/5/libgcc.a
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^
#	strip $@

clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
