/*
 * LEVEE, or Captain Video;  A vi clone
 *
 * Copyright (c) 1982-2007 David L Parsons
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, without or
 * without modification, are permitted provided that the above
 * copyright notice and this paragraph are duplicated in all such
 * forms and that any documentation, advertising materials, and
 * other materials related to such distribution and use acknowledge
 * that the software was developed by David L Parsons (orc@pell.portland.or.us).
 * My name may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */
/*
 * os2 (and bound) interface for levee (Microsoft C)
 */
#include "levee.h"

#if OS2

#include <signal.h>
#include <glob.h>

#define INCL_DOS
#include <os2.h>

int PROC
min(a,b)
int a,b;
{
    return (a>b) ? b : a;
}

int PROC
max(a,b)
int a,b;
{
    return (a<b) ? b : a;
}

PROC
strput(s)
char *s;
{
    write(1, s, strlen(s));
}

/* get a key, mapping certain control sequences
 */
PROC
getKey()
{
    register c;

    c = getch();

    if (c == 0 || c == 0xe0)
	switch (c=getch()) {
	case 'K': return LTARROW;
	case 'M': return RTARROW;
	case 'H': return UPARROW;
	case 'P': return DNARROW;
	case 'I': return 'U'-'@';	/* page-up */
	case 'Q': return 'D'-'@';	/* page-down */
	default : return 0;
	}
    return c;
}


/* don't allow interruptions to happen
 */
PROC
nointr()
{
    signal(SIGINT, SIG_IGN);
} /* nointr */


/* have ^C do what it usually does
 */
PROC
allowintr()
{
    signal(SIGINT, SIG_DFL);
} /* allowintr */


/*
 * basename() returns the filename part of a pathname
 */
char *
basename(s)
register char *s;
{
    register char *p = s;
    
    for (p = s+strlen(s); p > s; --p)
	if (p[-1] == '/' || p[-1] == '\\' || p[-1] == ':')
	    return p;
    return s;
} /* basename */


/*
 * glob() expands a wildcard, via calls to DosFindFirst/Next()
 * and pathname retention.
 */
char *
glob(path, dta)
char *path;
struct glob_t *dta;
{
    static char path_bfr[256];		/* full pathname to return */
    static char *file_part;		/* points at file - for filling */
    static char isdotpattern;		/* looking for files starting with . */
    static char isdotordotdot;		/* special case . or .. */
    static struct glob_t *dta_bfr;	/* pointer to desired dta */
    static FILEFINDBUF ffb;		/* DOS & OS/2 dta */
    static int dir;			/* directory handle */

    register st;			/* status from DosFindxxx */
    int count=1;			/* how many files to find */

    if (path) {
	/* when we start searching, save the path part of the filename in
	 * a safe place.
	 */
	strcpy(path_bfr, path);
	file_part = basename(path_bfr);

	/* set up initial parameters for DosFindFirst()
	 */
	dta_bfr = dta;
	dir = HDIR_SYSTEM;
	
	if (isdotpattern = (*file_part == '.'))
	    /* DosFindFirst() magically expands . and .. into their
	     * directory names.  Admittedly, there are cases where
	     * this can be useful, but this is not one of them. So,
	     * if we find that we're matching . and .., we just
	     * special-case ourselves into oblivion to get around
	     * this particular bit of DOS silliness.
	     */
	    isdotordotdot = (file_part[1] == 0 || file_part[1] == '.');
	else
	    isdotordotdot = 0;

	st = DosFindFirst(path, &dir, 0x16, &ffb, sizeof ffb, &count, 0L);
    }
    else
	st = DosFindNext(dir, &ffb, sizeof ffb, &count);

    while (st == 0 && count > 0) {
	/* Unless the pattern has a leading ., don't include any file
	 * that starts with .
	 */
	if (ffb.achName[0] == '.' && !isdotpattern) {
	    count = 1;
	    st = DosFindNext(dir, &ffb, sizeof ffb, &count);
	}
	else {
	    /* found a file - affix the path leading to it, then return
	     * a pointer to the (static) buffer holding the path+the name.
	     */
	    strlwr(ffb.achName);	/* DOS & OS/2 are case-insensitive */

	    if (dta_bfr) {
		memcpy(&dta_bfr->wr_date, &ffb.fdateLastWrite, sizeof(short));
		memcpy(&dta_bfr->wr_time, &ffb.ftimeLastWrite, sizeof(short));
		if (isdotordotdot)
		    strcpy(dta_bfr->name, file_part);
		else {
		    strncpy(dta_bfr->name,
			    ffb.achName, sizeof(dta_bfr->name)-1);
		    dta_bfr->name[sizeof(dta_bfr->name)-1] = 0;
		}
		dta_bfr->size   = ffb.cbFile;
		dta_bfr->attrib = ffb.attrFile;
	    }
	    if (!isdotordotdot)
		strcpy(file_part, ffb.achName);
	    return path_bfr;
	}
    }
    /* nothing matched
     */
    if (path && isdotordotdot) {
	/* must be at root, so statting dot will most likely fail.  Fake a
	 * dta.
	 */
	if (dta_bfr) {
	    memset(dta_bfr, 0, sizeof *dta_bfr);
	    dta_bfr->attrib = 0x10;
	    dta_bfr->name[0] = '.';
	}
	return path_bfr;
    }
    return (char*)0;
} /* glob */
#endif
