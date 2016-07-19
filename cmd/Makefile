TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

all: ../lib/ulib.a
	$(MAKE) -C old all
	$(MAKE) -C simple all
#	$(MAKE) -C elvis all

../lib/ulib.a:
	$(MAKE) -C ../lib ulib.a

clean:
	$(MAKE) -C old clean
	$(MAKE) -C simple clean
	$(MAKE) -C elvis clean
	rm -rf ../fs/bin/*
