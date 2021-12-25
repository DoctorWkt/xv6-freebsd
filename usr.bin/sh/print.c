/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include <sys/types.h>
#include <sys/param.h>	/* for HZ */
#include "defs.h"	/* includes sys/types.h if not already included */

#ifndef HZ
#define	HZ	60
#endif

union bg_nmbr { unsigned long u; char *x; };
char numbuf[3*sizeof(union bg_nmbr)];

/* printing and io conversion */

void
newline(void)
{
	prc(NL);
}

void
blank(void)
{
	prc(SP);
}

void
prp(void)
{
	if ((flags & prompt) == 0 && cmdadr) {
		prs(cmdadr);
		prs(colon);
	}
}

void
prs(STRING s)
{
	ssize_t len = length(s)-1;
	if (len > 0)
		write(output, s, len);
}

void
prc(char c)
{
	if (c)
		write(output, &c, 1);
}

void
prt(long t)		/* t is time in clock ticks, not seconds */
{
	int hr, min, sec;

	t += HZ / 2;	/* round to nearest second */
	t /= HZ;
	sec = t % 60;
	t /= 60;
	min = t % 60;
	if ((hr = t / 60)) {
		prn(hr);
		prc('h');
	}
	prn(min);
	prc('m');
	prn(sec);
	prc('s');
}

void
prn(int n)
{
	itos(n);
	prs(numbuf);
}

void
itos(unsigned long u)
{
  unsigned long q = u;
  char *s = numbuf;
  do { ++s; q /= 10; } while (q);	/* handles q < 9 */
  *s = 0;
  do { *--s = '0' + (u % 10); u /= 10; } while(u);
}

int
stoi(STRING icp)
{
	unsigned int r = 0;
	unsigned char c, *cp = (unsigned char *)icp;

	for (; (c = (*cp -'0')) < 10; cp++)
		r = r*10 + c;
	if ((char *)cp == icp || (int)r < 0)
		failed(icp, badnum);
	return r;
}
