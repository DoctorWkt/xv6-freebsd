TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

UPROGS= md5

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

$(UPROGS): mddriver.o md5c.o ../../lib/libc.a $(LIBGCC)
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ $^
	strip $@

clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
