/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include	"defs.h"
#include	"sym.h"

static void	chksym(int);
static void	chkword(void);
static IOPTR	inout(IOPTR);
static TREPTR	item(BOOL);
static TREPTR	list(int);
static TREPTR	makelist(int type, TREPTR i, TREPTR r);
static void	prsym(int);
static int	skipnl(void);
static void	synbad(void);
static REGPTR	syncase(int);
static TREPTR	term(int);

/* ========	command line decoding	========*/

TREPTR
makefork(int flgs, TREPTR i)
{
	FORKPTR	t;

	t = (FORKPTR)getstak(FORKTYPE);
	t->forktyp = flgs | TFORK;
	t->forktre = i;
	t->forkio = 0;
	return (TREPTR)t;
}

static TREPTR
makelist(int type, TREPTR i, TREPTR r)
{
	LSTPTR	t = 0;

	if (i == 0 || r == 0) {
		synbad();
	} else {
		t = (LSTPTR)getstak(LSTTYPE);
		t->lsttyp = type;
		t->lstlef = i;
		t->lstrit = r;
	}
	return (TREPTR)t;
}

/*
 * cmd
 *	empty
 *	list
 *	list & [ cmd ]
 *	list [ ; cmd ]
 */

TREPTR
cmd(int sym, int flg)
{
	TREPTR	i, e;

	i = list(flg);

	if (wdval == NL) {
		if (flg & NLFLG) {
			wdval = ';';
			chkpr(NL);
		}
	} else if (i == 0 && (flg & MTFLG) == 0)
		synbad();

	switch (wdval) {
	case '&':
		if (i)
			i = makefork(FINT | FPRS | FAMP, i);
		else
			synbad();
		/* FALLTHROUGH */
	case ';':
		if ((e = cmd(sym, flg | MTFLG)))
			i = makelist(TLST, i, e);
		break;

	case EOFSYM:
		if (sym == NL)
			break;
		/* FALLTHROUGH */
	default:
		if (sym)
			chksym(sym);
	}
	return(i);
}

/*
 * list
 *	term
 *	list && term
 *	list || term
 */

static TREPTR
list(int flg)
{
	TREPTR	r;
	int b;

	r = term(flg);
	while (r && ((b = (wdval == ANDFSYM)) || wdval == ORFSYM))
		r = makelist((b? TAND: TORF), r, term(NLFLG));
	return(r);
}

/*
 * term
 *	item
 *	item |^ term
 */

static TREPTR
term(int flg)
{
	TREPTR	t;

	reserv++;
	if (flg & NLFLG)
		skipnl();
	else
		word();

	/* ^ is a relic from the days of UPPER CASE ONLY tty model 33s */
#ifdef WANT_HAT_AS_PIPE
#define eval_HAT wdval == '^'
#else
#define eval_HAT 0
#endif
	if ((t = item(TRUE)) && (eval_HAT || wdval == '|'))
		return makelist(TFIL, makefork(FPOU, t),
			makefork(FPIN | FPCL, term(NLFLG)));
	else
		return(t);
}

static REGPTR
syncase(int esym)
{
	skipnl();
	if (wdval == esym)
		return(0);
	else {
		REGPTR	r = (REGPTR)getstak(REGTYPE);

		r->regptr = 0;
		for (; ;) {
			wdarg->argnxt = r->regptr;
			r->regptr = wdarg;
			if (wdval || (word() != ')' && wdval != '|'))
				synbad();
			if (wdval == '|')
				word();
			else
				break;
		}
		r->regcom = cmd(0, NLFLG | MTFLG);
		if (wdval == ECSYM) {
			r->regnxt = syncase(esym);
		} else {
			chksym(esym);
			r->regnxt = 0;
		}
		return(r);
	}
}

/*
 * item
 *
 *	(cmd) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 *	if ... then ... else ... fi
 *	for ... while ... do ... done
 *	case ... in ... esac
 *	begin ... end
 */

static TREPTR
item(BOOL flag)
{
	TREPTR	tr;
	IOPTR	io;

	if (flag)
		io = inout((IOPTR)0);
	else
		io = 0;

	switch (wdval) {
	case CASYM:
		{
			SWPTR t = (SWPTR)getstak(SWTYPE);

			chkword();
			t->swarg = wdarg->argval;
			skipnl();
			chksym(INSYM | BRSYM);
			t->swlst = syncase(wdval == INSYM? ESSYM: KTSYM);
			t->swtyp = TSW;
			tr = (TREPTR)t;
			break;
		}

	case IFSYM:
		{
			int w;
			IFPTR t = (IFPTR)getstak(IFTYPE);

			t->iftyp = TIF;
			t->iftre = cmd(THSYM, NLFLG);
			t->thtre = cmd(ELSYM | FISYM | EFSYM, NLFLG);
			t->eltre = ((w = wdval) == ELSYM? cmd(FISYM, NLFLG):
				(w == EFSYM? (wdval = IFSYM, item(0)): 0));
			if (w == EFSYM)
				return (TREPTR)t;
			tr = (TREPTR)t;
			break;
		}

	case FORSYM:
		{
			FORPTR t = (FORPTR) getstak(FORTYPE);

			t->fortyp = TFOR;
			t->forlst = 0;
			chkword();
			t->fornam = wdarg->argval;
			if (skipnl() == INSYM) {
				chkword();
				t->forlst = (COMPTR)item(0);
				if (wdval != NL && wdval != ';')
					synbad();
				chkpr(wdval);
				skipnl();
			}
			chksym(DOSYM | BRSYM);
			t->fortre = cmd(wdval == DOSYM? ODSYM: KTSYM, NLFLG);
			tr = (TREPTR)t;
			break;
		}

	case WHSYM:
	case UNSYM:
		{
			WHPTR t = (WHPTR)getstak(WHTYPE);

			t->whtyp = (wdval == WHSYM? TWH: TUN);
			t->whtre = cmd(DOSYM, NLFLG);
			t->dotre = cmd(ODSYM, NLFLG);
			tr = (TREPTR)t;
			break;
		}

	case BRSYM:
		tr = (TREPTR)cmd(KTSYM, NLFLG);
		break;

	case '(':
		{
			PARPTR p = (PARPTR)getstak(PARTYPE);

			p->partre = cmd(')', NLFLG);
			p->partyp = TPAR;
			tr = (TREPTR)makefork(0, (TREPTR)p);
			break;
		}

	default:
		if (io == 0)
			return(0);

	case 0:
		{
			int keywd = 1;
			ARGPTR	argp;
			ARGPTR	*argtail, *argset = 0;
			COMPTR t = (COMPTR)getstak(COMTYPE);

			t->comio = io;		/* initial io chain */
			argtail = &t->comarg;
			while (wdval == 0) {
				argp = wdarg;
				if (wdset && keywd) {
					argp->argnxt = (ARGPTR)argset;
					argset = (ARGPTR *)argp;
				} else {
					*argtail = argp;
					argtail = &argp->argnxt;
					keywd = flags & keyflg;
				}
				word();
				if (flag)
					t->comio = inout(t->comio);
			}

			t->comtyp = TCOM;
			t->comset = (ARGPTR)argset;
			*argtail = 0;
			return (TREPTR)t;
		}
	}
	reserv++;
	word();
	if ((io = inout(io))) {
		tr = makefork(0, tr);
		tr->treio = io;
	}
	return(tr);
}

static int
skipnl(void)
{
	while (reserv++, word() == NL)
		chkpr(NL);
	return(wdval);
}

static IOPTR
inout(IOPTR lastio)
{
	int iof;
	char c;
	IOPTR iop;

	iof = wdnum;

	switch (wdval) {
	case DOCSYM:
		iof |= IODOC;
		break;

	case APPSYM:
	case '>':
		if (wdnum == 0)
			iof |= 1;
		iof |= IOPUT;
		if (wdval == APPSYM) {
			iof |= IOAPP;
			break;
		}

	case '<':
		if ((c = nextc(0)) == '&')
			iof |= IOMOV;
		else if (c == '>')
			iof |= IORDW;
		else
			peekc = c | MARK;
		break;

	default:
		return(lastio);
	}

	chkword();
	iop = (IOPTR)getstak(IOTYPE);
	iop->ioname = wdarg->argval;
	iop->iofile = iof;
	if (iof & IODOC) {
		iop->iolst = iopend;
		iopend = iop;
	}
	word();
	iop->ionxt = inout(lastio);
	return(iop);
}

static void
chkword(void)
{
	if (word())
		synbad();
}

static void
chksym(int sym)
{
	int x = sym&wdval;

	if (((x & SYMFLG)? x: sym) != wdval)
		synbad();
}

static void
prsym(int sym)
{
	if (sym & SYMFLG) {
		SYSPTR sp = reserved;

		while (sp->sysval && sp->sysval != sym)
			sp++;
		prs(sp->sysnam);
	} else if (sym == EOFSYM)
		prs(endoffile);
	else {
		if (sym & SYMREP)
			prc(sym);
		if (sym == NL)
			prs("newline");
		else
			prc(sym);
	}
}

static void
synbad(void)
{
	prp();
	prs(synmsg);
	if ((flags & ttyflg) == 0) {
		prs(atline);
		prn(standin->flin);
	}
	prs(colon);
	prc(LQ);
	if (wdval) 
		prsym(wdval);
	else
		prs(wdarg->argval);
	prc(RQ);
	prs(unexpected);
	newline();
	exitsh(SYNBAD);
}
