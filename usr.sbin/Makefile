all: ../lib/ulib.a ../lib/libc.a
	$(MAKE) -C mount  all install

../lib/libc.a:
	$(MAKE) -C ../lib libc.a

../lib/ulib.a:
	$(MAKE) -C ../lib ulib.a

clean:
	$(MAKE) -C mount  clean
	rm -f ../fs/sbin/*
