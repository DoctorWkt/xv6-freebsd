/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */

#include	<stdlib.h>

// xv6 doesn't have any environment
char * getenv(const char *name)
{
  return (char *)NULL;
}

#if 0
extern const char ***_penviron;

char *
getenv(const char *name)
{
	register const char **v = *_penviron;
	register const char *p, *q;

	if (v == NULL || name == NULL)
		return (char *)NULL;
	while ((p = *v++) != NULL) {
		q = name;
		while (*q && (*q == *p++))
			q++;
		if (*q || (*p != '='))
			continue;
		return (char *)p + 1;
	}
	return (char *)NULL;
}
#endif
