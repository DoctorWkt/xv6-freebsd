TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

ULIB = ulib.o usys.o printf.o umalloc.o

ulib.a: $(ULIB)
	$(AR) vr ulib.a ulib.o usys.o printf.o umalloc.o
	$(RANLIB) ulib.a

clean:
	rm -f *.o ulib.a
