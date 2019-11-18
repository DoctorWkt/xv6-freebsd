all: ../lib/ulib.a ../lib/libc.a
	$(MAKE) -C old    all install
#	$(MAKE) -C ed     all install
	$(MAKE) -C simple all install
#	$(MAKE) -C wish   all install
#	$(MAKE) -C levee  all install
	$(MAKE) -C cp     all install
	$(MAKE) -C md5    all install
#	$(MAKE) -C sed    all install
	$(MAKE) -C less   all install
#	$(MAKE) -C make   all install
	$(MAKE) -C sh     all install
#	$(MAKE) -C nroff  all install

../lib/libc.a:
	$(MAKE) -C ../lib libc.a

../lib/ulib.a:
	$(MAKE) -C ../lib ulib.a

clean:
	$(MAKE) -C old    clean
#	$(MAKE) -C ed     clean
	$(MAKE) -C simple clean
	$(MAKE) -C wish   clean
	$(MAKE) -C levee  clean
	$(MAKE) -C cp     clean
	$(MAKE) -C md5    clean
	$(MAKE) -C sed    clean
	$(MAKE) -C less   clean
	$(MAKE) -C make   clean
	$(MAKE) -C sh     clean
	$(MAKE) -C nroff  clean
	rm -f ../fs/bin/*
	rm -f ../fs/etc/init
