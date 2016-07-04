/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Id: strcmp.c,v 1.4 1994/06/24 11:56:43 ceriel Exp $ */

#include	<string.h>

int
strcmp(register const char *s1, register const char *s2)
{
	while (*s1 == *s2++) {
		if (*s1++ == '\0') {
			return 0;
		}
	}
	if (*s1 == '\0') return -1;
	if (*--s2 == '\0') return 1;
	return (unsigned char) *s1 - (unsigned char) *s2;
}
