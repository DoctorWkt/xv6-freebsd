/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */

#include	<stdlib.h>

int
abs(register int i)
{
	return i >= 0 ? i : -i;
}
