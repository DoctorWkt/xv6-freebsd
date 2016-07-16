TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

ULIB = ulib.o usys.o printf.o umalloc.o

all: ulib.a

ulib.a: $(ULIB)
	$(AR) vr ../ulib.a ulib.o usys.o printf.o umalloc.o
	$(RANLIB) ../ulib.a

clean:
	rm -f *.d *.o */*.o ../ulib.a
