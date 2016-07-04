/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Id: memcmp.c,v 1.6 1994/06/24 11:56:26 ceriel Exp $ */

#include	<string.h>

int
memcmp(const void *s1, const void *s2, size_t n)
{
	register const unsigned char *p1 = s1, *p2 = s2;

	if (n) {
		n++;
		while (--n > 0) {
			if (*p1++ == *p2++) continue;
			return *--p1 - *--p2;
		}
	}
	return 0;
}
