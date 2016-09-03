/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Header: strcpy.c,v 1.2 90/05/31 18:33:13 ceriel Exp $ */

#include	<string.h>

char *
strcpy(char *ret, register const char *s2)
{
	register char *s1 = ret;

	while ((*s1++ = *s2++))
		/* EMPTY */ ;

	return ret;
}
