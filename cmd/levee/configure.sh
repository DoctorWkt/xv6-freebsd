#! /bin/sh

# local options:  ac_help is the help message that describes them
# and LOCAL_AC_OPTIONS is the script that interprets them.  LOCAL_AC_OPTIONS
# is a script that's processed with eval, so you need to be very careful to
# make certain that what you quote is what you want to quote.

ac_help="
--use-termcap		Link with termcap instead of curses, if possible
--partial-install	Don\'t install the lv, lv(1) name links
--size=NNN		Use a NNN-byte edit buffer
--dos			compile for ms-dos or microsoft windows
--tos			compile for the Atari ST
--rmx			compile for RMX
--flexos		compile for FlexOS"

LOCAL_AC_OPTIONS='
case Z$1 in
Z--partial-install)
	    missing_lv=1;;
Z--dos)	    ac_os=DOS;;
Z--tos)     ac_os=ATARI=1;;
Z--flexos)  ac_os=FLEXOS=1;;
Z--rmx)	    ac_os=RMX;;
Z--size=*)  SIZE=$(echo Z$1 | sed -e 's/^Z--size=//') ;;
*)          ac_error=1;;
esac;shift'

# load in the configuration file
#
TARGET=levee
. ./configure.inc
AC_INIT $TARGET

# validate --size=
#
case X"${SIZE}" in
X[0-9][0-9]*)	 ;;
X[0-9][0-9]*[Ll]);;
X)               ;;
X*)		 AC_FAIL "--size=$SIZE is not a valid number" ;;
esac

AC_PROG_CC

if [ "$OS_DOS" ]; then
    AC_DEFINE	SIZE ${SIZE:-32000}
    AC_DEFINE	PROC	_fastcall
    AC_DEFINE	TTY_ANSI	1
    AC_CHECK_FUNCS	basename
elif [ "$OS_ATARI" ]; then
    AC_DEFINE	SIZE ${SIZE:-32000}
    AC_DEFINE	TTY_VT52	1
    AC_DEFINE	HAVE_BLKFILL	1
    AC_CHECK_FUNCS	basename
elif [ "$OS_FLEXOS" ]; then
    AC_DEFINE	SIZE ${SIZE:-256000}
    AC_CHECK_FUNCS	basename
else
    AC_DEFINE	SIZE ${SIZE:-256000}
    AC_DEFINE	OS_UNIX	1

    if AC_CHECK_HEADERS string.h; then
	# Assume a mainly ANSI-compliant world, where the
	# existance of string.h implies a memset() and strchr()
	AC_DEFINE HAVE_MEMSET	1
	AC_DEFINE HAVE_STRCHR	1
    else
	AC_CHECK_FUNCS memset
	AC_CHECK_FUNCS strchr
    fi

    # for basename
    if AC_CHECK_FUNCS basename; then
	AC_CHECK_HEADERS libgen.h
    fi

    if AC_CHECK_HEADERS signal.h; then
	# Assume a mainly sane world where the existance
	# of signal.h means that signal() exists
	AC_DEFINE HAVE_SIGNAL 1
    fi

    if [ "$USE_TERMCAP" ]; then
	LIBORDER="-ltermcap -lcurses -lncurses"
    else
	LIBORDER="-lcurses -lncurses -ltermcap"
    fi

    if AC_LIBRARY tgetent $LIBORDER; then
	AC_CHECK_HEADERS termcap.h || AC_FAIL "levee needs <termcap.h>"
	AC_DEFINE USE_TERMCAP	1
	# our -libtermcap might be (n)curses in disguise.  If so,
	# it might have a colliding mvcur() that we need to define
	# ourselves out from.
	AC_QUIET AC_CHECK_FUNCS mvcur && AC_DEFINE mvcur __mvcur
    else
	# have to use a local termcap
	AC_DEFINE TERMCAP_EMULATION	1
	AC_DEFINE USE_TERMCAP	1
    fi

    AC_CHECK_HEADERS termios.h && AC_CHECK_FUNCS tcgetattr
fi

if AC_PROG_LN_S && test -z "$missing_lv"; then
    AC_SUB NOMK ''
else
    AC_SUB NOMK '@#'
fi

AC_OUTPUT Makefile
