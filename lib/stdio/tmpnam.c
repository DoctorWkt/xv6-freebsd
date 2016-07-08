/*
 * tmpnam.c - create a unique filename
 */
/* $Header: tmpnam.c,v 1.4 91/02/26 09:28:39 ceriel Exp $ */

#include	<sys/types.h>
#include	<stdio.h>
#include	<string.h>
#include	"loc_incl.h"

pid_t _getpid(void);

char *
tmpnam(char *s) {
	static char name_buffer[L_tmpnam] = "/tmp/tmp.";
	static unsigned long count = 0;
	static char *name = NULL;

	if (!name) { 
		name = name_buffer + strlen(name_buffer);
		name = _i_compute((unsigned long)_getpid(), 10, name, 5);
		*name++ = '.';
		*name = '\0';
	}
	if (++count > TMP_MAX) count = 1;	/* wrap-around */
	*_i_compute(count, 10, name, 3) = '\0';
	if (s) return strcpy(s, name_buffer);
	else return name_buffer;
}
