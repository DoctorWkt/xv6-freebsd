/*
 * fileno .c - map a stream to a file descriptor
 */

#include	<stdio.h>

int
(fileno)(FILE *stream)
{
	return stream->_fd;
}
