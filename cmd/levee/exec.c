#include "levee.h"
#include "extern.h"
#include <string.h>
#include <stdlib.h>

VOID PROC undefine();
VOID PROC fixupline();
VOID PROC doinput();

/*
 * do a newline and set flags.
 */
#define exprintln()	(zotscreen=YES),println()

VOID PROC
plural(num,string)
int num;
char *string;
{
    printi(num);
    prints(string);
    if (num != 1)
	printch('s');
} /* plural */


VOID PROC
clrmsg()
{
    mvcur(-1,0);
    strput(CE);
} /* clrmsg */


VOID PROC
errmsg(msg)
char *msg;
{
    mvcur(-1,0);
    prints(msg);
    strput(CE);
} /* errmsg */


/* get a space-delimited token */
char *execstr;			/* if someone calls getarg in the	*/
				/* wrong place, death will come...	*/
char *PROC
getarg()
{
    char *rv;
    rv = execstr;
    while (*execstr && !isspace(*execstr))
	++execstr;
    if (*execstr) {
	*execstr++ = 0;
	while (isspace(*execstr))
	    ++execstr;
    }
    return (*rv) ? rv : NULL;
} /* getarg */


VOID PROC
version()
/* version: print which version of levee we are... */
{
    errmsg("levee (c)");prints(codeversion);
} /* version */


VOID PROC
args()
/* args: print the argument list */
{
    register int i;
    mvcur(-1,0);
    for (i=0; i < argc; i++) {
	if (curpos.x+strlen(argv[i]) >= COLS)
	    exprintln();
	else if (i > 0)
	    printch(' ');
	if (pc == i) {			/* highlight the current filename.. */
#if OS_ATARI|OS_FLEXOS
	    strput("\033p");
#else
	    printch('[');
#endif
	    prints(argv[i]);
#if OS_ATARI|OS_FLEXOS
	    strput("\033q");
#else
	    printch(']');
#endif
	}
	else
	    prints(argv[i]);
    }
} /* args */
    
VOID PROC
setcmd()
{
    bool no = NO,b;
#if 0
    int len,i;
#endif
    char *arg, *num;
    struct variable *vp;
    
    if ( (arg = getarg()) ) {
	do {
	    if (*arg != 0) {
		if ( (num = strchr(arg, '=')) ) {
		    b = NO;
		    *num++ = 0;
		}
		else {				/* set [no]opt */
		    b = YES;
		    if (arg[0]=='n' && arg[1]=='o') {
			arg += 2;
			no = NO;
		    }
		    else
			no = YES;
		}
		for(vp=vars;vp->u && strcmp(arg,vp->v_name)
				  && strcmp(arg,vp->v_abbr); vp++)
		    ;
		if (!vp->u || vp->v_flags & V_CONST) {
		    errmsg("Can't set ");
		    prints(arg);
		}
		else {
		    int j;
                    
                    if (b)
			if (vp->v_tipe == VBOOL)
			    vp->u->valu = no;
			else
			    goto badsettype;
		    else if (vp->v_tipe == VSTR) {
			if (vp->u->strp)
			    free(vp->u->strp);
			vp->u->strp = (*num) ? strdup(num) : NULL;
		    }
		    else
			if (*num && (j=atoi(num)) >= 0)
			    vp->u->valu = j;
			else {
		  badsettype:
			    errmsg("bad set type");
			    continue;
			}
		    diddled |= vp->v_flags & V_DISPLAY;
		}
	    }
	} while ( (arg = getarg()) );
    }
    else {
	version(); exprintln();
	for(vp=vars;vp->u;vp++) {
	    switch (vp->v_tipe) {
		case VBOOL:
		    if (!vp->u->valu)
			prints("no");
		    prints(vp->v_name);
		    break;
		case VSTR:
		    if (!vp->u->strp)
			prints("no ");
		    prints(vp->v_name);
		    if (vp->u->strp) {
			mvcur(-1,10);
			prints("= ");
			prints(vp->u->strp);
		    }
		    break;
		default:
		    prints(vp->v_name);
		    mvcur(-1,10);
		    prints("= ");
		    printi(vp->u->valu);
		    break;
	    }
	    exprintln();
	}
    }
} /* setcmd */


/* print a macro */
VOID PROC
printone(i)
int i;
{
    if (i >= 0) {
	exprintln();
	printch(mbuffer[i].token);
	mvcur(-1,3);
	if (movemap[(unsigned int)(mbuffer[i].token)] == INSMACRO)
	    prints("!= ");
	else
	    prints(" = ");
	prints(mbuffer[i].m_text);
    }
} /* printone */


/* print all the macros */
VOID PROC
printall()
{
    int i;
    for (i = 0; i < MAXMACROS; i++)
	if (mbuffer[i].token != 0)
	    printone(i);
} /* printall */


/* :map ch text */
VOID PROC
map(insert)
bool insert;
{
    char *macro, c;
    int i;
    		/* get the macro */
    if ((macro=getarg()) == NULL) {
	printall();
	return;
    }
    if (strlen(macro) > 1) {
	errmsg("macros must be one character");
	return;
    }
    c = macro[0];
    if (*execstr == 0)
	printone(lookup(c));
    else {
	if ((i = lookup(0)) < 0)
	    errmsg("macro table full");
	else if (c == ESC || c == ':') {
	    errmsg("can't map ");
	    printch(c);
	}
	else if (*execstr != 0) {
	    undefine(lookup(c));
	    mbuffer[i].token = c;
	    mbuffer[i].m_text = strdup(execstr);
	    mbuffer[i].oldmap = movemap[(unsigned int)c];
	    if (insert)
		movemap[(unsigned int)c] = INSMACRO;
	    else
		movemap[(unsigned int)c] = SOFTMACRO;
	}
    }
} /* map */


VOID PROC
undefine(i)
int i;
{
    char *p;
    if (i >= 0) {
	movemap[(unsigned int)(mbuffer[i].token)] = mbuffer[i].oldmap;
	mbuffer[i].token = 0;
	p = mbuffer[i].m_text;
	free(p);
	mbuffer[i].m_text = 0;
    }
} /* undefine */


int PROC
unmap()
{
    int i;
    char *arg;
    
    if ( (arg=getarg()) ) {
	if (strlen(arg) == 1) {
	    undefine(lookup(*arg));
	    return YES;
	}
	if (strcmp(arg,"all") == 0) {
	    for (i = 0; i < MAXMACROS; i++)
		if (mbuffer[i].token != 0)
			undefine(i);
	    return YES;
	}
    }
    return NO;
} /* unmap */


/* return argument # of a filename */
int PROC
findarg(name)
register char *name;
{
    int j;
    for (j = 0; j < argc; j++)
	if (strcmp(argv[j],name) == 0)
	    return j;
    return -1;
} /* findarg */


/* add a filename to the arglist */
int PROC
addarg(name)
register char *name;
{
    int where;
    if ((where = findarg(name)) < 0)
	return doaddwork(name, &argc, &argv);
    return where;
} /* addarg */


/* get a file name argument (substitute alt file for #) */
char * PROC
getname()
{
    extern int wilderr;
#if OS_ATARI
    extern int mapslash;
    register char *p;
#endif
    register char *name;

    if ( (name = getarg()) ) {
	if ( 0 == strcmp(name,"#") ) {
	    if (*altnm)
		name = altnm;
	    else {
		errmsg("no alt name");
		wilderr++;
		return NULL;
	    }
	}
#if OS_ATARI
	if (mapslash)
	    for (p=name; *p; p++)
		if (*p == '/')
		    *p = '\\';
#endif
    }
    return name;
} /* getname */


/* CAUTION: these make exec not quite recursive */
int  high,low;		/* low && high end of command range */
bool affirm;		/* cmd! */
/* s/[src]/dst[/options] */
/* s& */
VOID PROC
cutandpaste()
{
    bool askme  = NO,
	 printme= NO,
	 glob   = NO;
    int newcurr = -1;
    int oldcurr = curr;
    int  num;
    char delim;
    register char *ip;
    register char *dp;
    
    zerostack(&undo);
    ip = execstr;
    if (*ip != '&') {
	delim = *ip;
	ip = makepat(1+ip,delim);			/* get search */
	if (ip == NULL)
	    goto splat;
	dp = dst;
	while (*ip && *ip != delim) {
	    if (*ip == '\\' && ip[1] != 0)
		*dp++ = *ip++;
	    *dp++ = *ip++;
	}
	*dp = 0;
	if (*ip == delim) {
	    while (*++ip)
		switch (*ip) {
		    case 'q':
		    case 'c': askme = YES;	break;
		    case 'g': glob = YES;	break;
		    case 'p': printme= YES;	break;
		}
	}
    }
    if (*lastpatt == 0) {
splat:	errmsg("bad substitute");
	return;
    }
    fixupline(bseekeol(curr));
    num = 0;
    do {
	low = chop(low, &high, NO, &askme);
	if (low > -1) {
	    diddled = YES;
	    num++;
	    if (printme) {
		exprintln();
		writeline(-1,-1,bseekeol(low));
	    }
	    if (newcurr < 0)
		newcurr = low;
	    if (!glob)
		low = 1+fseekeol(low);
	}
    } while (low >= 0);
    if (num > 0) {
	exprintln();
	plural(num," substitution");
    }
    fixupline((newcurr > -1) ? newcurr : oldcurr);
} /* cutandpaste */


/* quietly read in a file (and mark it in the undo stack)
 */
int PROC
insertfile(FILE *f, int insert, int at, int *fsize)
{
    int high,
	onright,
	rc=0;

    onright = (bufmax-at);
    high = SIZE-onright;

    if ( insert && (onright > 0) )
	 moveright(&core[at], &core[high], onright);

    rc = addfile(f, at, high, fsize);

    if ( (rc == 0) && (*fsize < 0) ) {
	rc = -1;
	*fsize=0;
    }
    if ( insert ) {
	if ( *fsize ) 
	    insert_to_undo(&undo, at, *fsize);
	modified = YES;
	if (onright > 0)
	    moveleft(&core[high], &core[at+(*fsize)], onright);
    }
    diddled = YES;
    return rc;
} /* insertfile */



VOID PROC
inputf(fname, newbuf)
register char *fname;
bool newbuf;
{
    FILE *f;
    int fsize,		/* bytes read in */
	rc;

    if (newbuf)
	readonly = NO;
	
    zerostack(&undo);

    if ( newbuf ) {
	modified = NO;
	low = 0;
    }
    else {
	fixupline(bseekeol(curr));
    }
    
    printch('"');
    prints(fname);
    prints("\" ");
    if ((f=fopen(fname, "r")) == NULL) {
	prints("[No such file]");
	fsize = 0;
	if (newbuf)
	    newfile = YES;
    }
    else {
	rc = insertfile(f, !newbuf, low, &fsize);
	fclose(f);

	if ( rc > 0 )
	    plural(fsize, " byte");
	else if ( rc < 0 )
	    prints("[read error]");
	else {
	    prints("[overflow]");
	    readonly=1;
	}
	if (newbuf) newfile = NO;
    }
    if (newbuf) {
	fillchar(contexts, sizeof(contexts), -1);
	bufmax = fsize;
    }
    if (*startcmd) {
	count = 1;
	if (*findparse(startcmd,&curr,low) != 0 || curr < 0)
	    curr = low;
	*startcmd = 0;
    }
    else
	curr = low;
} /* inputf */


/* Change a file's name (for autocopy). */
VOID PROC
backup(name)
char *name;
{
    char back[80];
#if !OS_UNIX
    char *p;
#endif

    strcpy(back, name);
#if OS_UNIX
    strcat(back, "~");
#else
    p = strrchr(basename(back), '.');
    if (p)
	strcpy(1+p, ",bkp");
    else
	strcat(back, ".bkp");
#endif
    
    unlink(back);
    rename(name, back);
} /* backup */


bool PROC
outputf(fname)
char *fname;
{
    bool whole;
    FILE *f;
    int status;
    zerostack(&undo);		/* undo doesn't survive past write */
    if (high < 0)
	high = (low < 0) ? bufmax : (1+fseekeol(low));
    if (low < 0)
	low  = 0;
    printch('"');
    prints(fname);
    prints("\" ");
    whole = (low == 0 && high >= bufmax-1);
    if (whole && autocopy)
	backup(fname);
    if ( (f=fopen(fname, "w")) ) {
	status = putfile(f, low, high);
	fclose(f);
	if (status) {
	    plural(high-low," byte");
	    if (whole)
		modified = NO;
	    return(YES);
	}
	else {
	    prints("[write error]");
	    unlink(fname);
	}
    }
    else
	prints(fisro);
    return(NO);
} /* outputf */


int PROC
oktoedit(writeold)
/* check and see if it is ok to edit a new file */
int writeold;	/* automatically write out changes? */
{
    if (modified && !affirm) {
	if (readonly) {
	    errmsg(fisro);
	    return NO;
	}
	else if (writeold && *filenm) {
	    if (!outputf(filenm))
		return NO;
	    printch(',');
	}
	else {
	    errmsg(fismod);
	    return NO;
	}
    }
    return YES;
} /* oktoedit */


/* write out all || part of a file */
bool PROC
writefile()
{
    char *name;
    
    if ((name=getname()) == NULL)
	name = filenm;
    if (*name) {
	if (outputf(name)) {
	    addarg(name);
	    return YES;
	}
	else
	    strcpy(altnm, name);
    }
    else
	errmsg("no file to write");
    return NO;
}


VOID PROC
editfile()
{
    char *name = NULL;	/* file to edit */
    char **myargv;
    int myargc;
    int i, newpc;
    if ((name = getarg()) && *name == '+') {
	strcpy(startcmd, (name[1])?(1+name):"$");
	name = getarg();
    }
    myargc=0;
    if (name)
	do {
	    if (!expandargs(name, &myargc, &myargv))
		return;
	} while ( (name=getarg()) );
    if (myargc == 0) {
	if (*filenm)
	    name = filenm;
	else
	    errmsg("no file to edit");
    }
    else if ((newpc = addarg(myargv[0])) >= 0) {
	name = argv[pc = newpc];
	for (i=1; i < myargc && doaddwork(myargv[i], &argc, &argv) >= 0; i++)
	    ;
    }
    killargs(&myargc, &myargv);
    if (name && oktoedit(NO))
	doinput(name);
}


VOID PROC
doinput(name)
char *name;
{
    inputf(name, YES);
    strcpy(altnm, filenm);
    strcpy(filenm, name);
}


VOID PROC
toedit(count)
int count;
{
    if (count > 1) {
	printi(count);
	prints(" files to edit; ");
    }
}


VOID PROC
readfile()
{
    char *name;
    
    if ( (name=getarg()) )
	inputf(name,NO);
    else
	errmsg("no file to read");
}


VOID PROC
nextfile(prev)
bool prev;
{
    char *name = NULL;
    int newpc=pc,
	myargc=0;
    char **myargv;
    bool newlist = NO;
    
    if (prev == 0)
	while ( (name=getarg()) )
	    if (!expandargs(name, &myargc, &myargv))
		return;
    
    if (oktoedit(autowrite)) {
	if (prev || (myargc == 1 && strcmp(myargv[0],"-") == 0)) {
	    if (pc > 0) {
		newpc = pc-1;
	    }
	    else {
		prints("(no prev files)");
		goto killem;
	    }
	}
	else if (myargc == 0) {
	    if (pc < argc-1) {
		newpc = 1+pc;
	    }
	    else {
		prints("(no more files)");
		goto killem;
	    }
	}
	else if (myargc > 1 || (newpc = findarg(myargv[0])) < 0) {
	    toedit(myargc);
	    newpc = 0;
	    newlist++;
	}
	doinput(newlist ? myargv[0] : argv[newpc]);
	pc = newpc;
	if (newlist) {
	    killargs(&argc, &argv);
	    argc = myargc;
	    argv = myargv;
	}
	else
    killem: if (!prev)
		killargs(&myargc, &myargv);
    }
}


/*
 * set up low, high; set dot to low
 */
VOID PROC
fixupline(dft)
int dft;
{
    if (low < 0)
	low = dft;
    if (high < 0)
	high = fseekeol(low)+1;
    else if (high < low) {		/* flip high & low */
	int tmp;
	tmp = high;
	high = low;
	low = tmp;
    }
    if (low >= ptop && low < pend) {
	setpos(skipws(low));
	yp = setY(curr);
    }
    else {
	curr = low;
	diddled = YES;
    }
}


VOID PROC
whatline()
{
    printi(to_line((low < 0) ? (bufmax-1) : low));
    if (high >= 0) {
	printch(',');
	printi(to_line(high));
    }
}


VOID PROC
print()
{
    do {
	exprintln();
	writeline(-1, 0, low);
	low = fseekeol(low) + 1;
    } while (low < high);
    exprintln();
}

/* move to different line */
/* execute lines from a :sourced || .lvrc file */


bool PROC
do_file(fname,mode,noquit)
char *fname;
exec_type *mode;
bool *noquit;
{
    char line[120];
    FILE *fp, *fopen();
    
    if ((fp = fopen(fname,"r")) != NULL) {
	indirect = YES;
	while (fgets(line,120,fp) && indirect) {
	    strtok(line, "\n");
	    if (*line != 0)
		doexec(line,mode,noquit);
	}
	indirect = YES;
	fclose(fp);
	return YES;
    }
    return NO;
}


VOID PROC
doins(flag)
bool flag;
{
    int i;
    curr = low;
    exprintln();
    low = insertion(1,setstep[flag],&i,&i,NO)-1;
    if (low >= 0)
	curr = low;
    diddled = YES;
}


/* figure out a address range for a command */
char * PROC
findbounds(ip)
char *ip;
{
    ip = findparse(ip, &low, curr);	/* get the low address */
    if (low >= 0) {
	low = bseekeol(low);		/* at start of line */
	if (*ip == ',') {		/* high address? */
	    ip++;
	    count = 0;
	    ip = findparse(ip, &high, curr);
	    if (high >= 0) {
		high = fseekeol(high);
		return(ip);
	    }
	}
	else
	    return(ip);
    }
    return(0);
}


/* parse the command line for lineranges && a command */
int PROC
parse(inp)
char *inp;
{
    int j,k;
    char cmd[80];
    low = high = ERR;
    affirm = 0;
    if (*inp == '%') {
	moveright(inp, 2+inp, 1+strlen(inp));
	inp[0]='1';
	inp[1]=',';
	inp[2]='$';
    }
    while (isspace(*inp))
	++inp;
    if (strchr(".$-+0123456789?/`'", *inp))
	if (!(inp=findbounds(inp))) {
	    errmsg("bad address");
	    return ERR;
	}
    while (isspace(*inp))
	++inp;
    j = 0;
    while (isalpha(*inp))
	cmd[j++] = *inp++;
    if (*inp == '!') {
	if (j == 0)
	    cmd[j++] = '!';
	else
	    affirm++;
	inp++;
    }
    else if (*inp == '=' && j == 0)
	cmd[j++] = '=';
    while (isspace(*inp))
	++inp;
    execstr = inp;
    if (j==0)
	return EX_CR;
    for (k=0; excmds[k]; k++)
	if (strncmp(cmd, excmds[k], j) == 0)
	    return k;
    return ERR;
}


/* inner loop of execmode */
VOID PROC
doexec(cmd, mode, noquit)
char *cmd;
exec_type *mode;
bool *noquit;
{
    int  what;
    bool ok;
    
    what = parse(cmd);
    ok = YES;
    if (diddled) {
	lstart = bseekeol(curr);
	lend = fseekeol(curr);
    }
    switch (what) {
	case EX_QUIT:				/* :quit */
	    if (affirm || what == lastexec || !modified)
		*noquit = NO;
	    else
		errmsg(fismod);
	    break;
	case EX_READ:				/* :read */
	    clrmsg();
	    readfile();
	    break;
	case EX_EDIT:				/* :read, :edit */
	    clrmsg();
	    editfile();
	    break;
	case EX_WRITE:
	case EX_WQ :				/* :write, :wq */
	    clrmsg();
	    if (readonly && !affirm)
		prints(fisro);
	    else if (writefile() && what==EX_WQ)
		*noquit = NO;
	    break;
	case EX_PREV:
	case EX_NEXT:				/* :next */
	    clrmsg();
	    nextfile(what==EX_PREV);
	    break;
	case EX_SUBS:				/* :substitute */
	    cutandpaste();
	    break;
	case EX_SOURCE:				/* :source */
	    if ((cmd = getarg()) && !do_file(cmd, mode, noquit)) {
		errmsg("cannot open ");
		prints(cmd);
	    }
	    break;
	case EX_XIT:
	    clrmsg();
	    if (modified) {
		if (readonly) {
		    prints(fisro);
		    break;
		}
		else if (!writefile())
		    break;
	    }

	    if (!affirm && (argc-pc > 1)) {	/* any more files to edit? */
		printch('(');
		plural(argc-pc-1," more file");
		prints(" to edit)");
	    }
	    else
		*noquit = NO;
	    break;
	case EX_MAP:
	    map(affirm);
	    break;
	case EX_UNMAP:
	    ok = unmap();
	    break;
	case EX_FILE:				/* :file */
	    if ( (cmd=getarg()) ) {		/* :file name */
		strcpy(altnm, filenm);
		strcpy(filenm, cmd);
		pc = addarg(filenm);
	    }
	    wr_stat();
	    break;
	case EX_SET:				/* :set */
	    setcmd();
	    break;
	case EX_CR:
	case EX_PR:				/* :print */
	    fixupline(bseekeol(curr));
	    if (what == EX_PR)
		print();
	    break;
	case EX_LINE:				/* := */
	    whatline();
	    break;
	case EX_DELETE:
	case EX_YANK:				/* :delete, :yank */
	    yank.lines = YES;
	    fixupline(lstart);
	    zerostack(&undo);
	    if (what == EX_DELETE)
		ok = deletion(low,high);
	    else
		ok = doyank(low,high);
	    diddled = YES;
	    break;
	case EX_PUT:				/* :put */
	    fixupline(lstart);
	    zerostack(&undo);
	    ok = putback(low, &high);
	    diddled = YES;
	    break;
	case EX_VI:				/* :visual */
	    *mode = E_VISUAL;
	    if (*execstr) {
		clrmsg();
		nextfile(NO);
	    }
	    break;
	case EX_EX:
	    *mode = E_EDIT;			/* :execmode */
	    break;
	case EX_INSERT:
	case EX_OPEN:				/* :insert, :open */
	    if (indirect)
		ok = NO;		/* kludge, kludge, kludge!!!!!!!!!! */
	    else {
		zerostack(&undo);
		fixupline(lstart);
		doins(what == EX_OPEN);
	    }
	    break;
	case EX_CHANGE:				/* :change */
	    if (indirect)
		ok = NO;		/* kludge, kludge, kludge!!!!!!!!!! */
	    else {
		zerostack(&undo);
		yank.lines = YES;
		fixupline(lstart);
		if (deletion(low,high))
		    doins(YES);
		else
		    ok = NO;
	    }
	    break;
	case EX_UNDO:				/* :undo */
	    low = fixcore(&high);
	    if (low >= 0) {
		diddled = YES;
		curr = low;
	    }
	    else ok = NO;
	    break;
	case EX_ARGS:				/* :args */
	    args();
	    break;
	case EX_VERSION:			/* version */
	    version();
	    break;
	case EX_ESCAPE:			/* shell escape hack */
	    zotscreen = YES;
	    exprintln();
	    if (*execstr) {
#if TTY_ZTERM
		zclose();
#endif
#if OS_FLEXOS|OS_UNIX
		fixcon();
#else
		allowintr();
#endif
		system(execstr);
#if OS_FLEXOS|OS_UNIX
		initcon();
#else
		nointr();
#endif
	    }
	    else
		prints("incomplete shell escape.");
	    break;
	case EX_REWIND:
	    clrmsg();
	    if (argc > 0 && oktoedit(autowrite)) {
		pc = 0;
		doinput(argv[0]);
	    }
	    break;
	default:
	    prints(":not an editor command.");
	    break;
    }
    lastexec = what;
    if (!ok) {
	errmsg(excmds[what]);
	prints(" error");
    }
}
