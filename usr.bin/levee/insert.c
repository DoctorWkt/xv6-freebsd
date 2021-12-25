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

int PROC
insertion(count, openflag, dp, yp, visual)
int count, openflag, *dp, *yp;
bool visual;
{
    char cmd, c;
    int rp;		/* number of spaces to diddle */
    int	ts, ss;		/* tabs && spaces to insert */
    register int cp;	/* current position */
    int i;		/* random index */
    int	endd;		/* last open place */
    register int rsize;	/* size of upper buffer */
    int	currDLE = 0;	/* what DLE is now */
    int	len;		/* full insert size */
    bool Dflag;

    if (openflag != 0) {	/* opening a line above || below */
	if (openflag<0 && bufmax>0 && curr<bufmax) {
	    curr = 1+lend;
	    if (visual)
		zwrite("\n", 1);
	}
	else {			/* open above current line */
	    (*yp)--;
	    curr = lstart;
	}
	if (autoindent)
	    currDLE = findDLE(lstart, &i, skipws(lstart),0);
	if (visual) {
#if TTY_VT52
	    if (OL) {
#else
	    if (OL && (*yp) < LINES-2) {
#endif
		strput(OL);
		(*yp)++;
		curpos.y = *yp;
	    }
	    else {
		mvcur(1+(*yp), 0);
		strput(CE);
	    }
	}
	mvcur(-1, currDLE);
    }
    else {
	if (autoindent)
	    currDLE = findDLE(lstart, &i, curr, 0);
	if (curr == i) {
	    if (!delete_to_undo(&undo, lstart, i-lstart))
		return(-1);
	    curr = lstart;
	}
    }

    rsize = (bufmax-curr);		/* amount of stuff above curr */
    endd = SIZE - rsize;		/* split the buffer */
    if (rsize > 0)
	moveright(&core[curr], &core[endd], rsize);

    cp = curr;
    do {				/* Insert loop */
	Dflag = (cp==0 || core[cp-1]==EOL);
	do {
	    if (Dflag)
		while ((cmd=peekc()) == '' || cmd == '') {
		    if (readchar() == '')
			currDLE = min(COLS,currDLE+shiftwidth);
		    else
			currDLE = max(0,currDLE-shiftwidth);
		    mvcur(-1, currDLE);
		}
	} while (!(c = line(core, cp, endd-1, &len)));
	if (Dflag && (len > 0 || c == ESC)) {
	    /* makeDLE : optimize leading whitespace for insert */
	    currDLE = findDLE(cp, &rp, cp+len, currDLE);
	    if (rp > cp) {
		len -= (rp-cp);
		moveleft(&core[rp], &core[cp], len);	/* squash whitespace */
	    }
	    if (currDLE > 0) {				/* create DLE indent */
		ts = currDLE / tabsize;
		ss = currDLE % tabsize;
		moveright(&core[cp], &core[cp+ts+ss], len);
		len += (ts+ss);
		fillchar(&core[cp   ], ts, TAB);
		fillchar(&core[cp+ts], ss, 32);
	    }
	}
	cp += len;
	if (c == EOL) {		/* Diddle-Diddle */
	    core[cp++] = EOL;		/* add in a \n */
	    strput(CE);				/* clear this line */
	    println();
	    if (visual) {
#if OS_RMX
		/* at OL at bottom kludge... */
		if (OL && (*yp) < LINES-2) {
#else
		if (OL) {
#endif
		    strput(OL);
		    (*yp)++;
		}
		else
		    strput(CE);
	    }
	    if (!autoindent)		/* reset currDLE? */
		currDLE = 0;
	    mvcur(-1, currDLE);
	}
    } while (c != ESC && cp <= endd-INSSIZE);
    *dp = cp;					/* start display here */

    if (count > 1) {				/* repeated insertion */
	len = cp-curr;
	if ((count-1)*len < endd-cp)
	    for (i = 1;i <count;i++) {
		moveright(&core[cp-len], &core[cp], len);
		cp += len;
	    }
    }

    if (openflag != 0		/* open or insert at end of buffer */
	    || (rsize < 1 && cp > curr && core[cp-1] != EOL))
	core[cp++] = EOL;
    len = cp-curr;

    if (rsize > 0)	/* if not at end of buffer, stitch things together */
	moveleft(&core[endd], &core[cp], rsize);
    insert_to_undo(&undo, curr, len);
    core[bufmax] = EOL;
    return(cp);
}
