/*
 * freopen.c - open a file and associate a stream with it
 */
/* $Header: freopen.c,v 1.8 90/08/28 13:44:48 eck Exp $ */

#if	defined(_POSIX_SOURCE)
#include	<sys/types.h>
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	"loc_incl.h"

#define	PMODE		0666

FILE *
freopen(const char *name, const char *mode, FILE *stream)
{
	register int i;
	int rwmode = 0, rwflags = 0;
	int fd, flags = stream->_flags & (_IONBF | _IOFBF | _IOLBF | _IOMYBUF);

	(void) fflush(stream);				/* ignore errors */
	(void) close(fileno(stream));

	switch(*mode++) {
	case 'r':
		flags |= _IOREAD;	
		rwmode = O_RDONLY;
		break;
	case 'w':
		flags |= _IOWRITE;
		rwmode = O_WRONLY;
		rwflags = O_CREAT | O_TRUNC;
		break;
	case 'a': 
		flags |= _IOWRITE | _IOAPPEND;
		rwmode = O_WRONLY;
		rwflags |= O_APPEND | O_CREAT;
		break;         
	default:
		return (FILE *)NULL;
	}

	while (*mode) {
		switch(*mode++) {
		case 'b':
			continue;
		case '+':
			rwmode = O_RDWR;
			flags |= _IOREAD | _IOWRITE;
			continue;
		/* The sequence may be followed by aditional characters */
		default:
			break;
		}
		break;
	}

	if ((rwflags & O_TRUNC)
	    || (((fd = open(name, rwmode)) < 0)
		    && (rwflags & O_CREAT))) {
		if (((fd = creat(name, PMODE)) < 0) && flags | _IOREAD) {
			(void) close(fd);
			fd = open(name, rwmode);
		}
	}

	if (fd < 0) {
		for( i = 0; i < FOPEN_MAX; i++) {
			if (stream == __iotab[i]) {
				__iotab[i] = 0;
				break;
			}
		}
		if (stream != stdin && stream != stdout && stream != stderr)
			free((void *)stream);
		return (FILE *)NULL;
	}

	stream->_count = 0;
	stream->_fd = fd;
	stream->_flags = flags;
	return stream;
}
