TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

.PHONY: all clean

all: bootblock entryother

bootblock: bootasm.S bootmain.c
	$(CC) $(CFLAGS) -fno-pic -O -c bootmain.c
	$(CC) $(CFLAGS) -fno-pic -c bootasm.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o bootblock.o bootasm.o bootmain.o
	$(OBJDUMP) -S bootblock.o > bootblock.asm
	$(OBJCOPY) -S -O binary -j .text bootblock.o bootblock
	./sign.pl bootblock

entryother: entryother.S
	$(CC) $(CFLAGS) -fno-pic -c entryother.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o bootblockother.o entryother.o
	$(OBJCOPY) -S -O binary -j .text bootblockother.o entryother
	$(OBJDUMP) -S bootblockother.o > entryother.asm

clean:
	rm -f *.o *.asm bootblock entryother
