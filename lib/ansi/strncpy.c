/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Header: strncpy.c,v 1.3 90/08/28 13:54:11 eck Exp $ */

#include	<string.h>

char *
strncpy(char *ret, register const char *s2, register size_t n)
{
	register char *s1 = ret;

	if (n>0) {
		while((*s1++ = *s2++) && --n > 0)
			/* EMPTY */ ;
		if ((*--s2 == '\0') && --n > 0) {
			do {
				*s1++ = '\0';
			} while(--n > 0);
		}
	}
	return ret;
}
