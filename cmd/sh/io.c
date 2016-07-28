/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"
#include <errno.h>
static unsigned int serial;

/* ========	input output and file copying ======== */

void
initf(UFD fd)
{
	FILE f = standin;

	f->fdes = fd;
	f->fsiz = ((flags&(oneflg|ttyflg)) == 0? BUFSIZ: 1);
	f->fnxt = f->fend = f->fbuf;
	f->feval = 0;
	f->flin = 1;
	f->feof = FALSE;
}

int
estabf(STRING s)
{
	FILE f = standin;

	f->fdes = -1;
	f->fend = length(s) + (f->fnxt = s);
	f->flin = 1;
	return f->feof = (s == 0);
}

void
push(FILE f)
{
	f->fstak = standin;
	f->feof = 0;
	f->feval = 0;
	standin = f;
}

int
pop(void)
{
	FILE f = standin;

	if (f->fstak) {
		if (f->fdes >= 0)
			close(f->fdes);
		standin = f->fstak;
		return(TRUE);
	} else
		return(FALSE);
}

void
chkpipe(int *pv)
{			 /* removed by Nikola Vladov */
	if (pipe(pv) < 0 /* || pv[INPIPE] < 0 || pv[OTPIPE] < 0 */)
		error(piperr);
}

int
chkopen(STRING idf, int o_flg)
{
	int rc =  open(idf, o_flg, 0666);
	if (rc < 0)
		failed(idf, badopen);
	return(rc);
}

/* renumber, actually */
void
myrename(int f1, int f2)
{
	if (f1 >= 0 && f2 >= 0 && f1 != f2) {
		if (dup2(f1, f2) < 0)
			//error(duperr);
			error("io dup error");
		close(f1);
		if (f2 == 0)
			ioset |= 1;
	}
}

int
tmpfil(void)
{	
	int fd;
	for (;;) {
		itos(serial++);
		movstr(numbuf, shtmpnam);
		fd = open(tmpout, O_CREAT|O_WRONLY|O_EXCL, 0600);
		if (fd >= 0 || errno != EEXIST) break;
	}
	if (fd < 0) failed(tmpout, badopen);
	return fd;
}

/* set by trim */
BOOL nosubst;

void
copy(IOPTR ioparg)
{
	int fd;
	char c, *ends;
	IOPTR iop;

	if ((iop = ioparg)) {
		copy(iop->iolst);
		ends = mactrim(iop->ioname);
		if (nosubst)
			iop->iofile &= ~IODOC;
		fd = tmpfil();
		iop->ioname = cpystak(tmpout);
		iop->iolst = iotemp;
		iotemp = iop;

		for (; ;) {
			staktop=stakbot;
			chkpr(NL);
			while (c = (nosubst? readc(): nextc(*ends)),!eolchar(c))
				pushstak(c);
			zerostak();
			if (eof || eq(stakbot, ends))
				break;
			pushstak(NL);
			write(fd, stakbot, staktop - stakbot);
		}
		staktop=stakbot;
		close(fd);
	}
}
