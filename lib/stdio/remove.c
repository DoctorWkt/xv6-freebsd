/*
 * remove.c - remove a file
 */

#include	<stdio.h>

int _unlink(const char *path);

int
remove(const char *filename) {
	return _unlink(filename);
}
