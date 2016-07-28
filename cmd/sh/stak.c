#include "defs.h"

char *staktop=0;
char *stakbot=0;
char *stakend=0;
char *stakbas=0;

/* ============== alloc in stak ============== */

static int brkincr = BRKINCR;

void
pushstak(char c) {
	if (staktop >= stakend) {
		setbrk(brkincr);
		brkincr += 256;
	}
	*staktop++ = c;
}

char *
getstak (int asize)	/* allocate requested stack */
{
	char	*oldstak = stakbot;
	int	size = align(int, asize, BYTESPERWORD);
	staktop = (stakbot += size);

	size = staktop - stakend;
	if (size >= 0)
		setbrk(max(brkincr, size));

	return oldstak;
}

char *
endstak ()
{
	pushstak(0);
	return getstak(staktop-stakbot);
}

void
tdystak (char *x)	/* try to bring stack back to x */
{
	while ((char *)stakbsy > x) {
		shfree(stakbsy);
		stakbsy = stakbsy->word;
	}  
	staktop = stakbot = max(x, stakbas);
	rmtemp((struct ionod *)x);
}

#define MINSTAK		(BRKINCR + BRKINCR/2)
void
stakchk(void)
{
	ssize_t len = (stakbas - stakend) + MINSTAK;
	if (len < -(int)SBRK_offset) setbrk(len);
	brkincr = BRKINCR;
}

char *
cpystak(char *x)
{
	char *tmp = getstak(length(x));
	movstr(x, tmp);
	return tmp;
}

char *
appstak(char *x)
{
	while (*x)
		pushstak(*x++);
	zerostak();
	return x;
}
