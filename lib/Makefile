include ../Makefile.common

all: ulib.a libc.a

ulib.a:
	$(MAKE) -C ulib all

libc.a:
	$(MAKE) -C gen all
	$(MAKE) -C i386 all
	$(MAKE) -C locale all
	$(MAKE) -C regex all
	$(MAKE) -C stdlib all
	$(MAKE) -C string all
	$(MAKE) -C stdio all
	$(MAKE) -C xv6 all
	$(AR) vr libc.a gen/*.o i386/*.o locale/*.o regex/*.o \
		stdlib/*.o string/*.o stdio/*.o xv6/*.o
	$(RANLIB) libc.a

libgcc.a:
	$(MAKE) -C libgcc all
	$(AR) vr libgcc.a libgcc/*.o
	$(RANLIB) libgcc.a

clean:
	$(MAKE) -C gen clean
	$(MAKE) -C i386 clean
	$(MAKE) -C locale clean
	$(MAKE) -C regex clean
	$(MAKE) -C stdlib clean
	$(MAKE) -C string clean
	$(MAKE) -C other clean
	$(MAKE) -C stdio clean
	$(MAKE) -C ulib clean
	$(MAKE) -C xv6 clean
	rm -f *.d *.o */*.o *.a
