TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

UPROGS= oldcat\
	echo\
	forktest\
	oldgrep\
	init\
	kill\
	ln\
	oldls\
	mkdir\
	oldrm\
	sh\
	stressfs\
	usertests\
	oldwc\
	zombie

all: ../../lib/ulib.a $(UPROGS)

install: all
	strip $(UPROGS)
	cp $(UPROGS) ../../fs/bin

../../lib/ulib.a:
	$(MAKE) -C ../../lib ulib.a

# Cancel the .c -> output implicit rule
%: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%: %.o ../../lib/ulib.a
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

forktest: forktest.o ../../lib/ulib.a
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o forktest forktest.o ../../lib/ulib.a


clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
