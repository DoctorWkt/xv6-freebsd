TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

UPROGS= mount

all: ../../lib/libc.a ../../lib/ulib.a ../../fs/bin $(UPROGS)

install: all
	strip $(UPROGS)
	cp $(UPROGS) ../../fs/sbin

../../fs/bin:
	mkdir -p ../../fs/sbin

../../lib/libc.a:
	$(MAKE) -C ../../lib libc.a

../../lib/ulib.a:
	$(MAKE) -C ../../lib ulib.a

# Cancel the .c -> output implicit rule
%: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(UPROGS): mount.o ../../lib/ulib.a $(LIBGCC)
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^

clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
