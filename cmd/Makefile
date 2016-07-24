all: ../lib/ulib.a ../lib/libc.a
	$(MAKE) -C old    all install
	$(MAKE) -C simple all install
	$(MAKE) -C less   all install
	$(MAKE) -C levee  all install
	$(MAKE) -C make   all install
	$(MAKE) -C wish   all install

../lib/libc.a:
	$(MAKE) -C ../lib libc.a

../lib/ulib.a:
	$(MAKE) -C ../lib ulib.a

clean:
	$(MAKE) -C old    clean
	$(MAKE) -C simple clean
	$(MAKE) -C less   clean
	$(MAKE) -C levee  clean
	$(MAKE) -C make   clean
	$(MAKE) -C wish   clean
	rm -f ../fs/bin/*
	rm -f ../fs/etc/init
