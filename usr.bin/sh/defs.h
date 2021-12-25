/*
 *	UNIX shell
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <unistd.h>  /* on 9, declares lseek, etc. if after sys/types.h */
#include <fcntl.h>
#include <limits.h>

#include "shfeatures.h"
#include "mac.h"
#include "mode.h"
#include "name.h"
#include "stak.h"
#include "ctype.h"

#ifdef PLAN9
/* APE's unistd.h doesn't declare these */
char	*brk(char *addr);
char	*sbrk(int incr);
#else
/* non-Plan-9 compilers don't have these built-in */
#define USED(x) (void)x
#define SET(x)
#endif

/* error exits from various parts of shell */
#define ERROR	1
#define SYNBAD	2
#define SIGFAIL 3
#define SIGFLG	0200

/* command tree */
#define FPRS	020
#define FINT	040
#define FAMP	0100
#define FPIN	0400
#define FPOU	01000
#define FPCL	02000
#define FCMD	04000
#define COMMSK	017

#define TCOM	0
#define TPAR	1
#define TFIL	2
#define TLST	3
#define TIF	4
#define TWH	5
#define TUN	6
#define TSW	7
#define TAND	8
#define TORF	9
#define TFORK	10
#define TFOR	11

/* execute table */
#define SYSSET	1
#define SYSCD	2
#define SYSEXEC	3
#define SYSLOGIN 4
#define SYSTRAP	5
#define SYSEXIT	6
#define SYSSHFT 7
#define SYSWAIT	8
#define SYSCONT 9
#define SYSBREAK 10
#define SYSEVAL 11
#define SYSDOT	12
#define SYSRDONLY 13
#define SYSTIMES 14
#define SYSXPORT 15
#define SYSNULL 16
#define SYSREAD 17
#define SYSTST	18
#define	SYSUMASK	19

/* used for input and output of shell */
#define INIO	(_POSIX_OPEN_MAX-2)
#define OTIO	(_POSIX_OPEN_MAX-1)

/* io nodes */
#define USERIO	INIO
#define IOUFD	(16 <<1)-1	/* mask for UNIX file descriptor number */
#define IODOC	(16 <<1)
#define IOPUT	(32 <<1)
#define IOAPP	(64 <<1)
#define IOMOV	(128<<1)
#define IORDW	(256<<1)
#define INPIPE	0
#define OTPIPE	1

/* arg list terminator */
#define ENDARGS	0

#define attrib(n,f)	((n)->namflg |= (f))
/* #define round(a,b)	(((int)((ADR(a)+(b))-1)) & ~((b)-1)) */
#define align(t,a,b)	(((t)(a) + (b-1)) & ~(b-1))
#define closepipe(x)	(close(x[INPIPE]), close(x[OTPIPE]))
#define eq(a,b)		(cf(a, b) == 0)
#define max(a,b)	((a)>(b)? (a): (b))

/* temp files and io */
UFD	output;
int ioset;
IOPTR	iotemp;		/* files to be deleted sometime */
IOPTR	iopend;		/* documents waiting to be read at NL */

/* substitution */
int dolc;
STRING	*dolv;
DOLPTR	argfor;
ARGPTR	gchain;

/* stack */
#define	BLK(x)	((BLKPTR)(x))
#define	BYT(x)	((BYTPTR)(x))
#define	STK(x)	((STKPTR)(x))
#define	ADR(x)	((char*)(x))

/* string constants */
extern MSG	atline;
extern MSG	readmsg;
extern MSG	colon;
extern MSG	minus;
extern MSG	nullstr;
extern MSG	sptbnl;
extern MSG	unexpected;
extern MSG	endoffile;
extern MSG	synmsg;
extern MSG	duperr;

/* name tree and words */
extern SYSTAB	reserved[];
int wdval;
int wdnum;
ARGPTR	wdarg;
int wdset;
BOOL	reserv;

/* prompting */
extern MSG	stdprompt;
extern MSG	supprompt;
extern MSG	profile;

/* built in names */
extern NAMNOD	fngnod;
extern NAMNOD	ifsnod;
extern NAMNOD	homenod;
extern NAMNOD	mailnod;
extern NAMNOD	pathnod;
extern NAMNOD	ps1nod;
extern NAMNOD	ps2nod;

/* special names; defined here */
extern MSG	flagadr;
STRING	cmdadr;
STRING	exitadr;
STRING	dolladr;
STRING	pcsadr;
STRING	pidadr;

extern MSG	defpath;

/* names always present */
extern MSG	mailname;
extern MSG	homename;
extern MSG	pathname;
extern MSG	fngname;
extern MSG	ifsname;
extern MSG	ps1name;
extern MSG	ps2name;

/* transput */
extern char tmpout[];
STRING shtmpnam;		/* defined here */

FILE	standin;

#define input	(standin->fdes)
#define eof	(standin->feof)

int peekc;
STRING	comdiv;			/* defined here */
extern MSG	devnull;

/* flags */
#define		noexec	01
#define		intflg	02
#define		prompt	04
#define		setflg	010
#define		errflg	020
#define		ttyflg	040
#define		forked	0100
#define		oneflg	0200
#define		rshflg	0400
#define		waiting	01000
#define		stdflg	02000
#define		execpr	04000
#define		readpr	010000
#define		keyflg	020000
#define		exportflg 040000	/* -a: export all */
int flags;

/* error exits from various parts of shell */
jmp_buf		subshell;
jmp_buf		errshell;

#define MINTRAP	0
#ifdef WANT_MAXTRAP
#define MAXTRAP WANT_MAXTRAP
#else
#define MAXTRAP	17
#endif

#define TRAPSET	2
#define SIGSET	4
#define SIGMOD	8

void		fault();
BOOL		trapnote;
extern STRING		trapcom[];
extern BOOL		trapflg[];

/* name tree and words */
extern STRING	*environ;
extern char numbuf[];
extern MSG	export;
extern MSG	readonly;

/* execflgs */
extern int exitval;
extern BOOL execbrk;
extern int loopcnt;
extern int breakcnt;

/* messages */
extern MSG	mailmsg;
extern MSG	coredump;
extern MSG	badopt;
extern MSG	badparam;
extern MSG	badsub;
extern MSG	nospace;
extern MSG	nostack;
extern MSG	notfound;
extern MSG	badtrap;
extern MSG	baddir;
extern MSG	badshift;
extern MSG	illegal;
extern MSG	restricted;
extern MSG	execpmsg;
extern MSG	notid;
extern MSG	wtfailed;
extern MSG	badcreate;
extern MSG	piperr;
extern MSG	badopen;
extern MSG	badnum;
extern MSG	arglist;
extern MSG	txtbsy;
extern MSG	toobig;
extern MSG	badexec;
extern MSG	notfound;
extern MSG	badfile;

/* function prototypes */
int	any(char c, STRING s);
void	assign(NAMPTR n, STRING v);
void	assnum(STRING *p, int i);
void	await(int i);
void	blank(void);
int	builtin(int, STRING *);
STRING	catpath(STRING path, STRING name);
int	cf(STRING s1, STRING s2);
int	chkopen(STRING idf, int flags);
void	chkpipe(int *pv);
void	chkpr(char eor);
void	chktrap(void);
void	clearup(void);
void	clrsig(int i);
TREPTR	cmd(int sym, int flg);
void	copy(IOPTR ioparg);
void	countnam(NAMPTR n);
int	create(STRING s);
void	dfault(NAMPTR n, STRING v);
void	done(void);
void	error(STRING s);
int	estabf(STRING s);
void	execa(STRING at[]);
void	execexp(STRING s, /*UFD*/ pth_ret f);
int	execute(TREPTR argt, int execflg, int *pf1, int *pf2);
void	exitset(void);
void	exitsh(int xno);
void	exname(NAMPTR n);
int	expand(STRING as, int rflg);
void	failed(STRING s1, STRING s2);
void	fault(int sig);
DOLPTR	freeargs(DOLPTR blk);
int	getarg(COMPTR ac);
STRING	getpath(STRING s);
void	getsig(int n);
int	gmatch(STRING s, STRING p);
int	ignsig(int n);
void	initf(UFD fd);
void	initio(IOPTR iop);
void	itos(unsigned long u);
int	length(STRING as);
NAMPTR	lookup(STRING nam);
STRING	macro(STRING as);
STRING	mactrim(STRING s);
STRING	make(STRING v);
void	makearg(STRING args);
TREPTR	makefork(int flgs, TREPTR i);
STRING	movstr(STRING a, STRING b);
void	mygetenv(void);
void	myrename(int, int);
void	namscan(void (*fn)(NAMPTR));
void	newline(void);
int	nextc(char);
void	oldsigs(void);
int	options(int argc, STRING *argv);
int	pathopen(STRING path, STRING name);
int	pop(void);
void	post(int pcsid);
void	postclr(void);
void	prc(char c);
void	printflg(NAMPTR n);
void	printnam(NAMPTR n);
void	prn(int n);
void	prp(void);
void	prs(STRING as);
void	prt(long t);
void	push(FILE);
void	pushnam(NAMPTR n);
int	readc(void);
int	readvar(STRING *names);
void	replace(STRING *a, STRING v);
void	rmtemp(IOPTR base);
STRING *scan(int argn);
void	setargs(STRING argi[]);
void	setlist(ARGPTR arg, int xp);
void	setname(STRING argi, int xp);
void	settmp(void);
void   *shalloc(size_t n);
void	shfree(void *p);
STRING *shsetenv(void);
void	sigchk(void);
void	stdsigs(void);
int	stoi(STRING icp);
void	subst(int in, int ot);
int	syslook(STRING w, SYSTAB syswds[]);
int	tmpfil(void);
void	trim(STRING at);
DOLPTR	useargs(void);
int	word(void);
void	Ldup(int fa, int fb);
void 	loadxv6env(void);
