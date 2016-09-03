/*
 * perror.c - print an error message on the standard error output
 */
/* $Header: perror.c,v 1.1 89/05/30 13:31:30 eck Exp $ */

#if	defined(_POSIX_SOURCE)
#include	<sys/types.h>
#endif
#include	<stdio.h>
#include	<errno.h>
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	"loc_incl.h"

ssize_t _write(int d, const char *buf, size_t nbytes);

void
perror(const char *s)
{
	char *p;
	int fd;

	p = strerror(errno);
	fd = fileno(stderr);
	fflush(stdout);
	fflush(stderr);
	if (s && *s) {
		_write(fd, s, strlen(s));
		_write(fd, ": ", 2);
	}
	_write(fd, p, strlen(p));
	_write(fd, "\n", 1);
}
