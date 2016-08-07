# make a printout
PRINT = runoff.list runoff.spec ../README toc.hdr toc.ftr

all: xv6.pdf

xv6.pdf: $(PRINT)
	./runoff
	ls -l xv6.pdf

clean:
	rm -rf fmt xv6.pdf
