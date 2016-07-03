TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

.PHONY: all clean

all: bootblock

bootblock: bootasm.S bootmain.c
	$(CC) $(CFLAGS) -fno-pic -O -c bootmain.c
	$(CC) $(CFLAGS) -fno-pic -c bootasm.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o bootblock.o bootasm.o bootmain.o
	$(OBJDUMP) -S bootblock.o > bootblock.asm
	$(OBJCOPY) -S -O binary -j .text bootblock.o bootblock
	./sign.pl bootblock

clean:
	rm -f *.o *.asm bootblock
