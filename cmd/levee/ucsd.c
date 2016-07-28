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

#ifndef moveleft

VOID PROC
moveleft(src,dest,length)
register char *src,*dest;
register int length;
{
    while (--length >= 0)
	*(dest++) = *(src++);
}

#endif /*moveleft*/

#ifndef moveright

VOID PROC
moveright(src,dest,length)
register char *src,*dest;
register int length;
{
    src = &src[length];
    dest = &dest[length];
    while (--length >= 0)
	*(--dest) = *(--src);
}

#endif /*moveright*/

#ifndef fillchar

VOID PROC
fillchar(src,length,ch)
register char *src,ch;
register int length;
{
    while (--length >= 0)
	*(src++) = ch;
}

#endif

int PROC
scan(length,tst,ch,src)
int length;
register char tst,ch,*src;
{
    register int inc,l;
    
    if (length < 0)
	inc = -1;
    else
	inc = 1;
    if (tst == '!') {
	for(l = ((int)inc)*length; l > 0; l--,src += (long)inc)
	    if (*src != ch)
		break;
    }
    else {
	for(l = ((int)inc)*length; l > 0; l--,src += (long)inc)
	    if (*src == ch)
		break;
    }
    return length-(inc*l);
}
