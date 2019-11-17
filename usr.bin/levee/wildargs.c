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
 * wildcard filename expanders for levee.
 */
#include "levee.h"
#include "extern.h"

#if OS_UNIX
# include <stdlib.h>
#elif !OS_RMX
# include <glob.h>
#endif


int wilderr, wildcard;

int PROC
expandargs(name, argcp, argvp)
char *name;
int *argcp;
char ***argvp;
{
#if OS_RMX|OS_UNIX
    wilderr = doaddwork(name, argcp, argvp) < 0;
#else
    register char *p;

    wilderr = 0;

    if (p=glob(name, (char*)0)) {
	do {
	    if (doaddwork(p, argcp, argvp) < 0) {
		wilderr++;
		break;
	    }
	} while (p=glob((char*)0, (char*)0));
    }
    else if (doaddwork(name, argcp, argvp) < 0)
	wilderr++;
#endif /*!OS_RMX*/
    if (wilderr)
	killargs(argcp, argvp);
    return !wilderr;
}

#define QUANTUM	10

int PROC
doaddwork(token,argcp,argvp)
char *token;
int *argcp;
char ***argvp;
{
    char **ap = *argvp;
    int ac = *argcp;
    int size;

    if ( ac%QUANTUM == 0) {	/* realloc more memory! */
	size = (QUANTUM+ac)*sizeof(char**);
	ap = (ac == 0)?malloc(size):realloc(ap, size);
	if (!ap) {
	    *argcp = 0;
	    goto memfail;
	}
    }
    if ( (ap[ac] = strdup(token)) ) {
#if OS_ATARI|OS_RMX|OS_FLEXOS
	strlwr(ap[ac]);		/* monocase filesystem */
#endif
	*argvp = ap;
	return (*argcp)++;
    }
memfail:
    errmsg("no memory");
    return -1;
}

VOID PROC
killargs(argcp, argvp)
int *argcp;
char ***argvp;
{
    int i;

    for (i=(*argcp)-1; i >= 0; i--)
	free((*argvp)[i]);
    if (*argcp)
	free(*argvp);
    *argcp = 0;
    *argvp = 0L;
}
