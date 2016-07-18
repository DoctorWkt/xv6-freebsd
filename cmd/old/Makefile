TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
CFLAGS = -fno-builtin -Wall -m32 -Werror -nostdinc -I$(INCLDIR)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

UPROGS= oldcat\
	echo\
	forktest\
	grep\
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
	mv $(UPROGS) ../../fs/bin

../../lib/ulib.a:
	$(MAKE) -C ../../lib ulib.a

# Cancel the .c -> output implicit rule
%: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%: %.o ../../lib/ulib.a
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	strip $@

forktest: forktest.o ../../lib/ulib.a
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o forktest forktest.o ../../lib/ulib.a


clean:
	rm -f $(UPROGS) *.asm *.o *.sym *.d
