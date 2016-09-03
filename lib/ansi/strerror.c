/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Header: strerror.c,v 1.3 90/08/28 13:53:31 eck Exp $ */

#include	<string.h>

/*
 * I don't know why, but X3J11 says that strerror() should be in declared
 * in <string.h>.  That is why the function is defined here.
 */
char *
strerror(register int errnum)
{
	extern const char *_sys_errlist[];
	extern const int _sys_nerr;

  	if (errnum < 0 || errnum >= _sys_nerr)
		return "unknown error";
	return (char *)_sys_errlist[errnum];
}
