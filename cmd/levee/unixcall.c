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
 * Unix interface for levee
 */
#include "levee.h"

#ifdef OS_UNIX

#include "extern.h"
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>

#if USE_TERMCAP3
#include <termcap.h>
#endif

int
min(a,b)
int a, b;
{
    return (a>b) ? b : a;
}

int
max(a,b)
int a, b;
{
    return (a<b) ? b : a;
}

void strput(s)
char *s;
{
#if USE_TERMCAP3
    if (s)
	tputs(s, 1, putchar);
#else
    if (s)
	write(1, s, strlen(s));
#endif
}

#if !HAVE_BASENAME
char *
basename(s)
char *s;
{
    char *rindex();
    char *p;

    if (p=strrchr(s,'/'))
	return 1+p;
    return s;
}
#endif


static int ioset = 0;
static struct termios old;

void
initcon()
{
    struct termios new;

    if (!ioset) {
	tcgetattr(0, &old);	/* get editing keys */

        Erasechar = old.c_cc[VERASE];
        eraseline = old.c_cc[VKILL];

        new = old;

	// new.c_iflag &= ~(IXON|IXOFF|IXANY|ICRNL|INLCR);
	new.c_lflag &= ~(ICANON|ISIG|ECHO);
	new.c_oflag = 0;

	tcsetattr(0, TCSANOW, &new);
        ioset=1;
    }
}

void
fixcon()
{
    if (ioset) {
	 tcsetattr(0, TCSANOW, &old);
         ioset = 0;
    }
}

int
getKey()
{
    unsigned char c[1];

    fflush(stdout);
    /* we're using Unix select() to wait for input, so lets hope that
     * all the Unices out there support select().  If your Unix doesn't,
     * you can make this work by replacing this select loop with:
     *
     *       while (read(0,c,1) != 1)
     *           ;
     *       return c[1];
     *
     * ... and watch your load-average peg.
     */
    while (read(0,c,1) != 1)
	;
    return c[0];
}
#endif
