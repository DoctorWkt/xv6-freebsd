/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Header: labs.c,v 1.1 89/05/16 13:08:11 eck Exp $ */

#include	<stdlib.h>

long
labs(register long l)
{
	return l >= 0 ? l : -l;
}
