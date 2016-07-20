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
#include "levee.h"
#include "extern.h"

/* read in a file -- return TRUE -- read file
			    FALSE-- file too big
*/

int PROC
addfile(f, start, endd, size)
FILE *f;
int start;
int endd, *size;
{
    register int chunk;

    chunk = read(fileno(f), core+start, (endd-start)-1);

    *size = chunk;
    return chunk < (endd-start)-1;
}


/* write out a file -- return TRUE if ok. */

bool PROC
putfile(f, start, endd)
register FILE *f;
register int start, endd;
{
    return write(fileno(f), core+start, endd-start) == (endd-start);
}
