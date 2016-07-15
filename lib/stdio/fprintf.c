/*
 * fprintf - write output on a stream
 */

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
