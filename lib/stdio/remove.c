/*
 * remove.c - remove a file
 */
/* $Header: remove.c,v 1.2 90/01/22 11:12:44 eck Exp $ */

#include	<stdio.h>

int _unlink(const char *path);

int
remove(const char *filename) {
	return _unlink(filename);
}
