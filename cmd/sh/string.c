/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"

/* ========	general purpose string handling ======== */

/* strcpy with arguments reversed and a more useful return value */
STRING
movstr(STRING a, STRING b)
{
	while ((*b++ = *a++))
		continue;
	return b-1;
}

/* simpler form of strchr with arguments reversed */
int
any(char c, STRING s)
{
	char d;

	while ((d = *s++))
		if (d == c)
			return(TRUE);
	return(FALSE);
}

/* strcmp */
int
cf(STRING s1, STRING s2)
{
	while (*s1++ == *s2)
		if (*s2++ == 0)
			return(0);
	return(*--s1 - *s2);
}

/* return size of as, including terminating NUL */
int 
length(STRING as)
{
	STRING s;

	if ((s = as))
		while (*s++);
	return(s - as);
}
