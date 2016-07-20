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

/* do a gotoXY -- allowing -1 for same row/column */

#if USE_TERMCAP | OS_ATARI

#define MAXCOLS 160

#if USE_TERMCAP
#include "termcap.i"
#endif

#else /*!(USE_TERMCAP | OS_ATARI)*/

#define MAXCOLS COLS

#endif

VOID PROC
mvcur(y,x)
int y,x;
{
#if TERMCAP_EMULATION || TTY_ANSI
    static char gt[30];
#endif
   
    if (y == -1)
	y = curpos.y;
    else
	curpos.y = y;
    if (y >= LINES)
	y = LINES-1;
    if (x == -1)
	x = curpos.x;
    else
	curpos.x = x;
    if (x >= COLS)
	x = COLS-1;

#if TERMCAP_EMULATION
    tgoto(gt,y,x);
    strput(gt);
#elif USE_TERMCAP
    strput( tgoto(CM, x, y) );
#elif TTY_ZTERM
    zgoto(x,y);
#elif TTY_ANSI
    {	register char *p = gt;		/* make a ansi gotoXY string */
	*p++ = 033;
	*p++ = '[';
	numtoa(p,1+y); p += strlen(p);
	*p++ = ';';
	numtoa(p,1+x); p += strlen(p);
	*p++ = 'H';
	WRITE_TEXT(1, gt, (p-gt));
    }
#elif TTY_VT52
    CM[2] = y+32;
    CM[3] = x+32;
    strput(CM);
#endif
}

VOID PROC
numtoa(str,num)
char *str;
int num;
{
    int i = 10;			/* I sure hope that str is 10 bytes long... */
    bool neg = (num < 0);

    if (neg)
	num = -num;

    str[--i] = 0;
    do{
	str[--i] = (num%10)+'0';
	num /= 10;
    }while (num > 0);
    if (neg)
	str[--i] = '-';
    moveleft(&str[i], str, 10-i);
}

VOID PROC
printi(num)
int num;
{
    char nb[10];
    register int size;
    
    numtoa(nb,num);
    size = min(strlen(nb),COLS-curpos.x);
    if (size > 0) {
	nb[size] = 0;
	zwrite(nb, size);
	curpos.x += size;
    }
}

VOID PROC
println()
{
    zwrite("\r\n", 2);
    curpos.x = 0;
    curpos.y = min(curpos.y+1, LINES-1);
}

/* print a character out in a readable form --
 *    ^<x> for control-<x>
 *    spaces for <tab>
 *    normal for everything else
 */

static char hexdig[] = "0123456789ABCDEF";

int PROC
format(out,c)
/* format: put a displayable version of c into out */
register char *out;
register unsigned c;
{
    if (c >= ' ' && c < '') {
    	out[0] = c;
    	return 1;
    }
    else if (c == '\t' && !list) {
	register int i;
	int size;

	for (i = size = tabsize - (curpos.x % tabsize);i > 0;)
	    out[--i] = ' ';
	return size;
    }
    else if (c < 128) {
    	out[0] = '^';
    	out[1] = c^64;
    	return 2;
    }
    else {
#if OS_DOS
	out[0] = c;
	return 1;
#else
	out[0] = '\\';
	out[1] = hexdig[(c>>4)&017];
	out[2] = hexdig[c&017];
	return 3;
#endif
    }
}

VOID PROC
printch(c)
char c;
{
    register int size;
    char buf[MAXCOLS];

    size = min(format(buf,c),COLS-curpos.x);
    if (size > 0) {
	buf[size] = 0;
	zwrite(buf, size);
	curpos.x += size;
    }
}

VOID PROC
prints(s)
char *s;
{
    int size,oxp = curpos.x;
    char buf[MAXCOLS+1];
    register int bi = 0;

    while (*s && curpos.x < COLS) {
    	size = format(&buf[bi],*s++);
    	bi += size;
    	curpos.x += size;
    }
    size = min(bi,COLS-oxp);
    if (size > 0) {
	buf[size] = 0;
	zwrite(buf, size);
    }
}

VOID PROC
writeline(y,x,start)
int y,x,start;
{
    int endd,oxp;
    register int size;
    char buf[MAXCOLS+1];
    register int bi = 0;
    
    endd = fseekeol(start);
    if (start==0 || core[start-1] == EOL)
	mvcur(y, 0);
    else
	mvcur(y, x);
    oxp = curpos.x;

    while (start < endd && curpos.x < COLS) {
    	size = format(&buf[bi],core[start++]);
    	bi += size;
    	curpos.x += size;
    }
    if (list) {
    	buf[bi++] = '$';
    	curpos.x++;
    }
    size = min(bi,COLS-oxp);
    if (size > 0) {
	buf[size] = 0;
	zwrite(buf, size);
    }
    if (curpos.x < COLS)
	strput(CE);
}

/* redraw && refresh the screen */

VOID PROC
refresh(y,x,start,endd,rest)
int y,x,start,endd;
bool rest;
{
    int sp;
    
#if OS_ATARI
    /* turn the cursor off */
    asm(" clr.l  -(sp)     ");
    asm(" move.w #21,-(sp) ");
    asm(" trap   #14       ");
    asm(" addq.l #6,sp     ");
#endif
    sp = start;
    while (sp <= endd) {
	writeline(y, x, sp);
	sp = 1+fseekeol(sp);
	y++;
	x = 0;
    }
    if (rest && sp >= bufmax)
	while (y<LINES-1) { /* fill screen with ~ */
	    mvcur(y, 0);
	    printch('~'); strput(CE);
	    y++;
	}
#if OS_ATARI
    /* turn the cursor back on */
    asm(" clr.w  -(sp)     ");
    asm(" move.w #1,-(sp)  ");
    asm(" move.w #21,-(sp) ");
    asm(" trap   #14       ");
    asm(" addq.l #6,sp     ");
#endif
}

/* redraw everything */

VOID PROC
redisplay(flag)
bool flag;
{
    if (flag)
	clrprompt();
    refresh(0, 0, ptop, pend, TRUE);
}
    
VOID PROC
scrollback(curr)
int curr;
{
    mvcur(0,0);		/* move to the top line */
    do {
	ptop = bseekeol(ptop-1);
	strput(UpS);
	writeline(0, 0, ptop);
    } while (ptop > curr);
    setend();
}

VOID PROC
scrollforward(curr)
int curr;
{
    do {
	writeline(LINES-1, 0, pend+1);
	zwrite("\n", 1);
	pend = fseekeol(pend+1);
	ptop = fseekeol(ptop)+1;
    } while (pend < curr);
}

/* find if the number of lines between top && bottom is less than dofscroll */

bool PROC
ok_to_scroll(top,bottom)
int top,bottom;
{
    int nl, i;
    
    nl = dofscroll;
    i = top;
    do
	i += 1+scan(bufmax-i,'=',EOL, &core[i]);
    while (--nl > 0 && i < bottom);
    return(nl>0);
}

VOID PROC
clrprompt()
{
    mvcur(LINES-1,0);
    strput(CE);
}

VOID PROC
prompt(toot,s)
bool toot;
char *s;
{
    if (toot)
	error();
    clrprompt();
    prints(s);
}
