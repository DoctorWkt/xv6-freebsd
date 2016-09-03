/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)isctype.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#define _ANSI_LIBRARY
#include <ctype.h>

#undef isalnum
int
isalnum(c)
	int c;
{
	return(__istype((c), (_A|_D)));
}

#undef isalpha
int
isalpha(c)
	int c;
{
	return (__istype((c), _A));
}

#undef isascii
int
isascii(c)
	int c;
{
	return((c & ~0x7F) == 0);
}

#undef isblank
int
isblank(c)
	int c;
{
	return (__istype((c), _B));
}

#undef iscntrl
int
iscntrl(c)
	int c;
{
	return (__istype((c), _C));
}

#undef isdigit
int
isdigit(c)
	int c;
{
	return (__isctype((c), _D));
}

#undef isgraph
int
isgraph(c)
	int c;
{
	return (__istype((c), _G));
}

#undef islower
int
islower(c)
	int c;
{
	return (__istype((c), _L));
}

#undef isprint
int
isprint(c)
	int c;
{
	return (__istype((c), _R));
}

#undef ispunct
int
ispunct(c)
	int c;
{
	return (__istype((c), _P));
}

#undef isspace
int
isspace(c)
	int c;
{
	return (__istype((c), _S));
}

#undef isupper
int
isupper(c)
	int c;
{
	return (__istype((c), _U));
}

#undef isxdigit
int
isxdigit(c)
	int c;
{
	return (__isctype((c), _X));
}

#undef toascii
int
toascii(c)
	int c;
{
	return (c & 0177);
}

#undef toupper
int
toupper(c)
	int c;
{
        return((c & _CRMASK) ? ___toupper(c) : _CurrentRuneLocale->mapupper[c]);
}

#undef tolower
int
tolower(c)
	int c;
{
        return((c & _CRMASK) ? ___tolower(c) : _CurrentRuneLocale->maplower[c]);
}
