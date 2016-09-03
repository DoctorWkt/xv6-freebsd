/*
 * fprintf - write output on a stream
 */
/* $Header: fprintf.c,v 1.3 89/12/18 15:01:54 eck Exp $ */

#include	<stdio.h>
#if 0
#include	<stdarg.h>
#endif
#include	"loc_incl.h"

int
fprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	int retval;
	
	va_start(ap, format);

	retval = _doprnt (format, ap, stream);

	va_end(ap);

	return retval;
}
