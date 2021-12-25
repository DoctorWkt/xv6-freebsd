/*-
 * Copyright (c) 1992 Diomidis Spinellis.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Diomidis Spinellis of Imperial College, University of London.
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
 *
 *	from: @(#)defs.h	8.1 (Berkeley) 6/6/93
 *	$Id: defs.h,v 1.2 1994/04/17 09:41:38 alm Exp $
 */

/*
 * Types of address specifications
 */
enum e_atype {
	AT_RE,					/* Line that match RE */
	AT_LINE,				/* Specific line */
	AT_LAST,				/* Last line */
};

/*
 * Format of an address
 */
struct s_addr {
	enum e_atype type;			/* Address type */
	union {
		u_long l;			/* Line number */
		regex_t *r;			/* Regular expression */
	} u;
};

/*
 * Substitution command
 */
struct s_subst {
	int n;					/* Occurrence to subst. */
	int p;					/* True if p flag */
	char *wfile;				/* NULL if no wfile */
	int wfd;				/* Cached file descriptor */
	regex_t *re;				/* Regular expression */
	int maxbref;				/* Largest backreference. */
	u_long linenum;				/* Line number. */
	char *new;				/* Replacement text */
};


/*
 * An internally compiled command.
 * Initialy, label references are stored in t, on a second pass they
 * are updated to pointers.
 */
struct s_command {
	struct s_command *next;			/* Pointer to next command */
	struct s_addr *a1, *a2;			/* Start and end address */
	char *t;				/* Text for : a c i r w */
	union {
		struct s_command *c;		/* Command(s) for b t { */
		struct s_subst *s;		/* Substitute command */
		u_char *y;			/* Replace command array */
		int fd;				/* File descriptor for w */
	} u;
	char code;				/* Command code */
	u_int nonsel:1;				/* True if ! */
	u_int inrange:1;			/* True if in range */
};

/*
 * Types of command arguments recognised by the parser
 */
enum e_args {
	EMPTY,			/* d D g G h H l n N p P q x = \0 */
	TEXT,			/* a c i */
	NONSEL,			/* ! */
	GROUP,			/* { */
	COMMENT,		/* # */
	BRANCH,			/* b t */
	LABEL,			/* : */
	RFILE,			/* r */
	WFILE,			/* w */
	SUBST,			/* s */
	TR			/* y */
};

/*
 * Structure containing things to append before a line is read
 */
struct s_appends {
	enum {AP_STRING, AP_FILE} type;
	char *s;
	size_t len;
};

enum e_spflag {
	APPEND,					/* Append to the contents. */
	REPLACE,				/* Replace the contents. */
};

/*
 * Structure for a space (process, hold, otherwise).
 */
typedef struct {
	char *space;		/* Current space pointer. */
	size_t len;		/* Current length. */
	int deleted;		/* If deleted. */
	char *back;		/* Backing memory. */
	size_t blen;		/* Backing memory length. */
} SPACE;

/*
 * Error severity codes:
 */
#define	FATAL		0	/* Exit immediately with 1 */
#define	ERROR		1	/* Continue, but change exit value */
#define	WARNING		2	/* Just print the warning */
#define	COMPILE		3	/* Print error, count and finish script */
#define	COMPILE2	3	/* Print error, count and finish script */
