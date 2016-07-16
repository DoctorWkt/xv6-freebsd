TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

ULIB = ulib.o usys.o printf.o umalloc.o libc.a

all: ulib.a libc.a

ulib.a:
	$(MAKE) -C ulib all

libc.a:
	$(MAKE) -C ansi all
	$(MAKE) -C stdio all
	$(MAKE) -C xv6 all
	$(AR) vr libc.a ansi/*.o stdio/*.o xv6/*.o
	$(RANLIB) libc.a

clean:
	$(MAKE) -C ansi clean
	$(MAKE) -C stdio clean
	$(MAKE) -C ulib clean
	$(MAKE) -C xv6 clean
	rm -f *.d *.o */*.o ulib.a libc.a
