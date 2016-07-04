TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

ULIB = ulib.o usys.o printf.o umalloc.o libc.a

ulib.a: $(ULIB)
	$(AR) vr ulib.a ulib.o usys.o printf.o umalloc.o
	$(RANLIB) ulib.a

libc.a:
	$(MAKE) -C ansi all
	$(AR) vr libc.a ansi/*.o
	$(RANLIB) libc.a

clean:
	rm -f *.o */*.o ulib.a libc.a
