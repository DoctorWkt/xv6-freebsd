TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

all: ../lib/ulib.a
	$(MAKE) -C old    all install
	$(MAKE) -C simple all install
	$(MAKE) -C less  all install
	$(MAKE) -C levee  all install
	$(MAKE) -C wish  all install

../lib/ulib.a:
	$(MAKE) -C ../lib ulib.a

clean:
	$(MAKE) -C old    clean
	$(MAKE) -C simple clean
	$(MAKE) -C less  clean
	$(MAKE) -C levee  clean
	$(MAKE) -C wish  clean
	rm -rf ../fs/bin/*
