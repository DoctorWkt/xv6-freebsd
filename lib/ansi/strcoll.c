/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Header: strcoll.c,v 1.2 90/08/28 13:53:23 eck Exp $ */

#include	<string.h>
#include	<locale.h>

int
strcoll(register const char *s1, register const char *s2)
{
	while (*s1 == *s2++) {
		if (*s1++ == '\0') {
			return 0;
		}
	}
	return *s1 - *--s2;
}
