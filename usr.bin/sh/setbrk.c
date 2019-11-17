/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include "defs.h"
#include "db_setbrk.h"

char *
setbrk(int incr)
{
	char *a;
	a = sbrk(incr + SBRK_offset);
	if (a == (char *)-1)
	  	error(nospace);
	stakend = a + incr;

	pr_info();
	return a;
}
