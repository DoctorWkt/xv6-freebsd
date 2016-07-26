all: ../lib/ulib.a ../lib/libc.a
	$(MAKE) -C old    all install
	$(MAKE) -C simple all install
	$(MAKE) -C cp     all install
#	$(MAKE) -C ls     all install

../lib/libc.a:
	$(MAKE) -C ../lib libc.a

../lib/ulib.a:
	$(MAKE) -C ../lib ulib.a

clean:
	$(MAKE) -C old    clean
	$(MAKE) -C simple clean
	$(MAKE) -C cp     clean
	$(MAKE) -C ls     clean
	rm -f ../fs/bin/*
	rm -f ../fs/etc/init
