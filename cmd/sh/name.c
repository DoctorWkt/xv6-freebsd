/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"

NAMNOD	ps2nod	 = { NIL, 	NIL, 	ps2name	  ,0,0,0 },
	fngnod	 = { NIL, 	NIL, 	fngname   ,0,0,0 },
	pathnod  = { NIL, 	NIL, 	pathname  ,0,0,0 },
	ifsnod	 = { NIL, 	NIL, 	ifsname   ,0,0,0 },
	ps1nod	 = { &pathnod, 	&ps2nod, ps1name  ,0,0,0 },
	homenod  = { &fngnod, 	&ifsnod, homename ,0,0,0 },
	mailnod  = { &homenod, 	&ps1nod, mailname ,0,0,0 };
NAMPTR		namep = &mailnod;


static BOOL	chkid(STRING);
static void	namwalk(NAMPTR np);
static STRING	staknam(NAMPTR np);

/* ========	variable and string handling	======== */

int
syslook(STRING w, SYSTAB syswds[])
{
	char first;
	STRING	s;
	SYSPTR	syscan;

	if (w == NIL)
		return 0;
	first = *w;
	for (syscan = syswds; (s = syscan->sysnam) != NIL; syscan++)
		if (first == *s && eq(w, s))
			return syscan->sysval;
	return 0;
}

void
setlist(ARGPTR arg, int xp)
{
	if (flags&exportflg)
		xp |= N_EXPORT;
	while (arg) {
		STRING s = mactrim(arg->argval);

		setname(s, xp);
		arg = arg->argnxt;
		if (flags & execpr) {
			prs(s);
			if (arg)
				blank();
			else
				newline();
		}
	}
}

void
setname(STRING argi, int xp)
{
	STRING argscan = argi;
	NAMPTR n;

	if (letter(*argscan)) {
		while (alphanum(*argscan))
			argscan++;
		if (*argscan == '=') {
			*argscan = 0;
			n = lookup(argi);
			*argscan++ = '=';
			attrib(n, xp);
			if (xp & N_ENVNAM)
				n->namenv = n->namval = argscan;
			else
				assign(n, argscan);
			return;
		}
	}
	failed(argi, notid);
}

void
replace(STRING *a, STRING v)
{
	shfree(*a);
	*a = make(v);
}

void
dfault(NAMPTR n, STRING v)
{
	if (n->namval == 0)
		assign(n, v);
}

void
assign(NAMPTR n, STRING v)
{
	if (n->namflg & N_RDONLY)
		failed(n->namid, wtfailed);
	else
		replace(&n->namval, v);
}

int
readvar(STRING *names)
{
	int rc = 0, nfd;
	char c;
	FILEBLK	fb;
	FILE f = &fb;
	NAMPTR	n = lookup(*names++);	/* done now to avoid storage mess */
	STKPTR	rel = (STKPTR)relstak();

	push(f);
	nfd = dup(0);
	if (nfd < 0)
		// error(duperr);
		error("name dup error");
	initf(nfd);
	if (lseek(0, 0L, 1) == -1)
		f->fsiz = 1;

	for (; ; ) {
		c = nextc(0);
		if ((*names && any(c, ifsnod.namval)) || eolchar(c)) {
			zerostak();
			assign(n, absstak(rel));
			setstak(rel);
			if (*names)
				n = lookup(*names++);
			else
				n = 0;
			if (eolchar(c))
				break;
		} else
			pushstak(c);
	}
	while (n) {
		assign(n, nullstr);
		if (*names)
			n = lookup(*names++);
		else
			n = 0;
	}

	if (eof)
		rc = 1;
	lseek(0, (long)(f->fnxt - f->fend), 1);
	pop();
	return rc;
}

void
assnum(STRING *p, int i)
{
	itos(i);
	replace(p, numbuf);
}

STRING
make(STRING v)
{
	STRING	p;

	if (v) {
		movstr(v, p = shalloc(length(v)));
		return(p);
	} else
		return(0);
}

NAMPTR
lookup(STRING nam)
{
	NAMPTR	nscan = namep;
	NAMPTR	*prev = 0;
	int LR;

	if (!chkid(nam))
		failed(nam, notid);
	while (nscan) {
		if ((LR = cf(nam, nscan->namid)) == 0)
			return(nscan);
		else if (LR < 0)
			prev = &nscan->namlft;
		else
			prev = &nscan->namrgt;
		nscan = *prev;
	}

	/* add name node */
	nscan = (NAMPTR)shalloc(sizeof * nscan);
	nscan->namlft = 0;
	nscan->namrgt = 0;
	nscan->namid = make(nam);
	nscan->namval = 0;
	nscan->namflg = N_DEFAULT;
	nscan->namenv = 0;
	return *prev = nscan;
}

static BOOL
chkid(STRING nam)
{
	char *cp = nam;

	if (!letter(*cp))
		return(FALSE);
	else
		while (*++cp)
			if (!alphanum(*cp))
				return(FALSE);
	return(TRUE);
}

static void (*namfn)(NAMPTR);

void
namscan(void (*fn)(NAMPTR))
{
	namfn = fn;
	namwalk(namep);
}

static void
namwalk(NAMPTR np)
{
	if (np) {
		namwalk(np->namlft);
		(*namfn)(np);
		namwalk(np->namrgt);
	}
}

void
printnam(NAMPTR n)
{
	STRING	s;

	sigchk();
	if ((s = n->namval)) {
		prs(n->namid);
		prc('=');
		prs(s);
		newline();
	}
}

static STRING
staknam(NAMPTR n)
{
	appstak(n->namid);
	pushstak('=');
	appstak(n->namval);
	return endstak();
}

void
exname(NAMPTR n)
{
	if (n->namflg & N_EXPORT) {
		shfree(n->namenv);
		n->namenv = make(n->namval);
	} else {
	  	shfree(n->namval);
		n->namval = make(n->namenv);
	}
}

void
printflg(NAMPTR n)
{
	if (n->namflg & N_EXPORT) {
		prs(export);
		blank();
	}
	if (n->namflg & N_RDONLY) {
		prs(readonly);
		blank();
	}
	if (n->namflg & (N_EXPORT | N_RDONLY)) {
		prs(n->namid);
		newline();
	}
}

void
mygetenv(void)
{
	STRING *e;
	loadxv6env();
	e = environ;

	while (*e) {
		/* scripts should not inherit IFS */
		if (!memcmp(*e, "IFS=", 4))
			continue;
		setname(*e++, N_ENVNAM);
	}
}

static int namec;

void
countnam(NAMPTR n)
{
	USED(n);
	namec++;
}

static STRING *argnam;

void
pushnam(NAMPTR n)
{
	if (n->namval)
		*argnam++ = staknam(n);
}

STRING *
shsetenv(void)
{
	STRING *er;

	namec = 0;
	namscan(countnam);
	argnam = er = (STRING *)getstak(namec * BYTESPERWORD + BYTESPERWORD);
	namscan(pushnam);
	*argnam++ = 0;
	return er;
}
