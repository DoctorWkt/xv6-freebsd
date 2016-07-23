TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common
CFLAGS+= -D_POSIX_SOURCE

all: beep.o charpick.o curs_set.o cursesio.o endwin.o flash.o initscr.o \
	longname.o move.o mvcursor.o newwin.o options.o overlay.o prntscan.o \
	refresh.o scrreg.o setterm.o tabsize.o termmisc.o unctrl.o update.o \
	waddch.o waddstr.o wbox.o wclear.o wclrtobot.o wclrtoeol.o wdelch.o \
	wdeleteln.o werase.o wgetch.o wgetstr.o windel.o winmove.o winsch.o \
	winscrol.o winsertln.o wintouch.o

clean:
	rm -f *.o
