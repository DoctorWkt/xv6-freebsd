/*
 * sprintf - print formatted output on an array
 */
/* $Header: sprintf.c,v 1.2 89/12/18 15:03:52 eck Exp $ */

#include	<stdio.h>
#include	<stdarg.h>
#include	<limits.h>
#include	"loc_incl.h"

int vsnprintf(char *s, size_t n, const char *format, va_list arg);

int
sprintf(char *s, const char *format, ...)
{
	va_list ap;
	int retval;

	va_start(ap, format);

	retval = vsnprintf(s, INT_MAX, format, ap);

	va_end(ap);

	return retval;
}

int
snprintf(char *s, size_t n, const char *format, ...)
{
	va_list ap;
	int retval;

	va_start(ap, format);

	retval = vsnprintf(s, n, format, ap);

	va_end(ap);

	return retval;
}
