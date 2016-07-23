include ../Makefile.common

all: ulib.a libc.a libcurses.a

ulib.a:
	$(MAKE) -C ulib all

libc.a:
	$(MAKE) -C ansi all
	$(MAKE) -C other all
	$(MAKE) -C regex all
	$(MAKE) -C stdio all
	$(MAKE) -C xv6 all
	$(AR) vr libc.a ansi/*.o other/*.o regex/*.o stdio/*.o xv6/*.o
	$(RANLIB) libc.a

libcurses.a:
	$(MAKE) -C curses all
	$(AR) vr libcurses.a curses/*.o
	$(RANLIB) libcurses.a

clean:
	$(MAKE) -C ansi clean
	$(MAKE) -C curses clean
	$(MAKE) -C other clean
	$(MAKE) -C regex clean
	$(MAKE) -C stdio clean
	$(MAKE) -C ulib clean
	$(MAKE) -C xv6 clean
	rm -f *.d *.o */*.o *.a
