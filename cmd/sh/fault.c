/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"

STRING	trapcom[MAXTRAP];
BOOL	trapflg[MAXTRAP];

/* ========	fault handling routines	   ======== */

void
fault(int sig)
{
	int flag;

	signal(sig, fault);		/* re-enable catching sig */
	if (sig == SIGALRM) {
		if (flags & waiting)
			done();
	} else {
		flag = (trapcom[sig]? TRAPSET: SIGSET);
		trapnote |= flag;
		trapflg[sig] |= flag;
	}
}

void
stdsigs(void)
{
	ignsig(SIGQUIT);
	getsig(SIGINT);
	getsig(SIGALRM);
}

int
ignsig(int n)
{
	int s;

	s = (signal(n, SIG_IGN) == SIG_IGN);
	if (s == 0)
		trapflg[n] |= SIGMOD;
	return s;
}

void
getsig(int i)
{
	if (trapflg[i] & SIGMOD || ignsig(i) == 0)
		signal(i, fault);
}

void
oldsigs(void)
{
	int i;
	STRING	t;

	for (i = MAXTRAP; i--; ) {
		t = trapcom[i];
		if (t == 0 || *t)
			clrsig(i);
		trapflg[i] = 0;
	}
	trapnote = 0;
}

void
clrsig(int i)
{
	shfree(trapcom[i]);
	trapcom[i] = 0;
	if (trapflg[i] & SIGMOD) {
		signal(i, fault);
		trapflg[i] &= ~SIGMOD;
	}
}

void
chktrap(void)
{
	/* check for traps */
	int i;
	STRING	t;

	trapnote &= ~TRAPSET;
	for (i = MAXTRAP; --i; )
		if (trapflg[i] & TRAPSET) {
			trapflg[i] &= ~TRAPSET;
			if ((t = trapcom[i])) {
				int savxit = exitval;

				execexp(t, (pth_ret)0);
				exitval = savxit;
				exitset();
			}
		}
}
