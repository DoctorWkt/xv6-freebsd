/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"
#include "sym.h"
#include "timeout.h"

UFD	output = 2;
static BOOL beenhere = FALSE;

#define	TMPNAM 7	/* offset to variable part of tmpout */
char	tmpout[8 + 5*(sizeof(pid_t) + sizeof(size_t))/2] = "/tmp/sh-";

FILEBLK	stdfile;
FILE	standin = &stdfile;

static void exfile(BOOL);

int
main(int c, STRING v[])
{
	int rflag = ttyflg;

#ifdef SH_SETUIDGID
	setgid(getgid());
	setuid(getuid());
#endif

	stdsigs();

	/* initialise storage allocation */
	blokbas = BLK(align(size_t, setbrk(BRKINCR), BYTESPERWORD));
	bloktop = blokbas;
	addblok(BRKINCR);

	/* set names from userenv */
	mygetenv();

	/* look for restricted */
#ifdef SH_RESTRICTED
	if (c > 0 && any('r', *v)) rflag = 0;
#endif

	/* look for options */
	dolc = options(c, v);
	if (dolc < 2)
		flags |= stdflg;
	if ((flags & stdflg) == 0)
		dolc--;
	dolv = v + c - dolc;
	dolc--;

	/* return here for shell file execution */
	setjmp(subshell);

	/* number of positional parameters */
	assnum(&dolladr, dolc);
	cmdadr = dolv[0];

	/* set up tmp files names */
	settmp();

	/* set pidname */
	replace(&pidadr, &tmpout[TMPNAM]);

	/* default ifs */
	dfault(&ifsnod, sptbnl);

	if ((beenhere++) == FALSE) {	/*? profile */
		if (*cmdadr == '-' &&
		    (input = pathopen(nullstr, profile)) >= 0) {
			exfile(rflag);
			flags &= ~ttyflg;
		}
		if (rflag == 0)
			flags |= rshflg;

		/* open input file if specified */
		if (comdiv) {
			estabf(comdiv);
			input = -1;
		} else {
			input = (flags & stdflg? 0: chkopen(cmdadr, 0));
			comdiv--;
		}
	} else {
		/* *execargs = dolv;	// for `ps' cmd */
	}

	exfile(0);
	done();
	return 0;
}

static void
exfile(BOOL prof)
{
	int userid;
#ifdef SH_MAIL
	long mailtime = 0;
	struct stat statb;
#endif

	/* move input */
	if (input > 0) {
		Ldup(input, INIO);
		input = INIO;
	}

	/* move output to safe place */
	if (output == 2) {
		int nfd = dup(2);

		if (nfd < 0)
			//error(duperr);
			error("main dup error");
		Ldup(nfd, OTIO);
		output = OTIO;
#ifdef PLAN9
		/*
		 * TODO: fix this.  Plan 9 APE needs it; Unix doesn't.
		 * Without it, we don't get "command: not found" messages
		 * on Plan 9.  APE has had problems with dup before.
		 */
		(void) fcntl(output, F_SETFD, 0);	/* no close-on-exec */
#endif
	}

	userid = getuid();

	/* decide whether interactive */
	if ((flags&intflg) ||
	    ((flags&oneflg) == 0 && isatty(output) && isatty(input))) {
		dfault(&ps1nod, (userid? stdprompt: supprompt));
		dfault(&ps2nod, readmsg);
		flags |= ttyflg | prompt;
		// ignsig(SIGKILL);
	} else {
		flags |= prof;
		flags &= ~prompt;
	}

	if (setjmp(errshell) && prof) {
		close(input);
		return;
	}

	/* error return here */
	loopcnt = breakcnt = peekc = 0;
	iopend = 0;
	if (input >= 0)
		initf(input);

	/* command loop */
	for (; ;) {
		tdystak(0);
		stakchk();		/* may reduce sbrk */
		exitset();
		if ((flags & prompt) && standin->fstak == 0 && !eof) {
#ifdef SH_MAIL
			if (mailnod.namval && stat(mailnod.namval, &statb) >= 0
			    && statb.st_size && statb.st_mtime != mailtime &&
			    mailtime)
				prs(mailmsg);
			mailtime = statb.st_mtime;
#endif
			prs(ps1nod.namval);
			alarm(TIMEOUT);
			flags |= waiting;
		}

		trapnote = 0;
		peekc = readc();
		if (eof)
			return;
		alarm(0);
		flags &= ~waiting;
		execute(cmd(NL, MTFLG), 0, NIL, NIL);
		eof |= (flags & oneflg);
	}
}

void
chkpr(char eor)
{
	if ((flags & prompt) && standin->fstak == 0 && eor == NL)
		prs(ps2nod.namval);
}

void
settmp(void)
{
	itos(getpid());
	shtmpnam = movstr(numbuf, &tmpout[TMPNAM]);
}

/* dup file descriptor fa to fb, close fa and set fb to close-on-exec */
void
Ldup(int fa, int fb)
{
	if (fa == fb) return;
	myrename(fa, fb);
	// fcntl(fb, F_SETFD, FD_CLOEXEC);
}
