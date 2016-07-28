/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"

char flagchar[] = {
	'x', 	'n', 	'v', 	't', 	's', 	'i', 	'e', 	'r', 	'k',
	'u', 	'a',	0
};
int flagval[]  = {
	execpr,	noexec,	readpr,	oneflg,	stdflg,	intflg, errflg,	rshflg,	keyflg,
	setflg,	exportflg, 0
};
char flagadr[sizeof flagchar/sizeof flagchar[0]];

static DOLPTR dolh;

static STRING *copyargs(STRING from[], int n);

/* ========	option handling	======== */

int
options(int argc, STRING *argv)
{
	STRING	cp, flagc, flagp;
	STRING	*argp = argv;

	if (argc > 1 && *argp[1] == '-') {
		cp = argp[1];
		flags &= ~(execpr|readpr);
		while (*++cp) {
			for (flagc = flagchar; *flagc && *flagc != *cp;
			     flagc++)
				continue;
			if (*cp == *flagc)
				flags |= flagval[flagc-flagchar];
			else if (*cp == 'c' && argc > 2 && comdiv == 0) {
				comdiv = argp[2];
				argp[1] = argp[0];
				argp++;
				argc--;
			} else
				failed(argv[1], badopt);
		}
		argp[1] = argp[0];
		argc--;
	}

	/* set up $- */
	flagp = flagadr;
	for (flagc = flagchar; *flagc; flagc++)
		if (flags & flagval[flagc-flagchar])
			*flagp++ = *flagc;
	*flagp = 0;
	return(argc);
}

void
setargs(STRING argi[])
{
	int argn = 0;
	STRING *argp = argi;

	/* count args */
	while (*argp++ != ENDARGS)
		argn++;

	/* free old ones unless on for loop chain */
	freeargs(dolh);
	dolh = (DOLPTR)copyargs(argi, argn);	/* sets dolv */
	assnum(&dolladr, dolc = argn - 1);
}

DOLPTR
freeargs(DOLPTR blk)
{
	STRING *argp;
	DOLPTR argr = 0, argblk;

	if ((argblk = blk)) {
		argr = argblk->dolnxt;
		if (--argblk->doluse == 0) {
			for (argp = (STRING *)argblk->dolarg; *argp != ENDARGS;
			     argp++)
				shfree(*argp);
			shfree(argblk);
		}
	}
	return(argr);
}

static STRING *
copyargs(STRING from[], int n)
{
	STRING *np = (STRING *)shalloc(sizeof(STRING *) * n + 3 * BYTESPERWORD);
	STRING *fp = from, *pp = np;

	((DOLPTR)np)->doluse = 1;	/* use count */
	dolv = np = (STRING *)((DOLPTR)np)->dolarg;

	while (n--)
		*np++ = make(*fp++);
	*np = ENDARGS;
	return(pp);
}

void
clearup(void)
{
	/* force `for' $* lists to go away */
	while ((argfor = freeargs(argfor)))
		continue;

	/* clean up io files */
	while (pop())
		continue;
}

DOLPTR
useargs(void)
{
	if (dolh) {
		dolh->doluse++;
		dolh->dolnxt = argfor;
		return argfor = dolh;
	} else
		return 0;
}
