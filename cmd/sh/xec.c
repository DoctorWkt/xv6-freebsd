/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"
#include "sym.h"

extern SYSTAB commands[];
int exitval;
BOOL execbrk;
int loopcnt;
int breakcnt;

static int parent;

/* ========	command execution	========*/

int
execute(TREPTR argt, int execflg, int *pf1, int *pf2)
{
	/* `stakbot' is preserved by this routine */
	TREPTR t;
	STKPTR sav = stakbot;
	
	sigchk();
	
	if ((t = argt) && execbrk == 0) {
		int treeflgs, oldexit, type, nfd;
		STRING	*com = 0;
		
		treeflgs = t->tretyp;
		type = treeflgs & COMMSK;
		oldexit = exitval;
		exitval = 0;

		switch (type) {
		case TCOM:
			{
				int argn, internal;
				STRING a1;
				ARGPTR schain = gchain;
				IOPTR io = t->treio;
				
				gchain = 0;
				argn = getarg((COMPTR)t);
				com = scan(argn);
				a1 = com[1];
				gchain = schain;

				if ((internal = syslook(com[0], commands)) ||
				    argn == 0)
					setlist(((COMPTR)t)->comset, 0);

				if (argn && (flags&noexec) == 0) {
					/* print command if execpr */
					if (flags & execpr) {
						argn = 0;
						prs(execpmsg);
						while (com[argn] != ENDARGS) {
							prs(com[argn++]);
							blank();
						}
						newline();
					}

					switch (internal) {
					case SYSDOT:
						if (a1) {
							pth_ret f;
							
							f.fd = pathopen(getpath(a1), a1);
							if (f.fd < 0)
								failed(a1, notfound);
							else
								execexp(0,f);
						}
						break;
						
					case SYSTIMES:
						{
							struct tms t;
							
							times(&t);
							prt(t.tms_cutime);
							blank();
							prt(t.tms_cstime);
							newline();
						}
						break;
						
					case SYSEXIT:
						exitsh(a1? stoi(a1): oldexit);
						
					case SYSNULL:
						io = 0;
						break;
						
					case SYSCONT:
						execbrk = -loopcnt;
						break;
						
					case SYSBREAK:
						if ((execbrk = loopcnt) && a1)
							breakcnt = stoi(a1);
						break;

					case SYSTRAP:
						if (a1) {
							BOOL clear;
							
							if ((clear = digit(*a1)) == 0)
								++com;
							while (*++com) {
								int i;
								
								if ((i = stoi(*com)) >= MAXTRAP)
									failed(*com, badtrap);
								else if (clear)
									clrsig(i);
								else {
									replace(&trapcom[i], a1);
									if (*a1)
										getsig(i);
									else
										ignsig(i);
								}
							}
						} else {		/* print out current traps */
							int i;
							
							for (i = 0; i < MAXTRAP; i++)
								if (trapcom[i]) {
									prn(i);
									prs(colon);
									prs(trapcom[i]);
									newline();
								}
						}
						break;

					case SYSEXEC:
						com++;
						initio(io);
						ioset = 0;
						io = 0;
						if (a1 == 0)
							break;
						/* FALLTHROUGH */

					case SYSLOGIN:
						flags |= forked;
						oldsigs();
						execa(com);
						done();

					case SYSCD:
						if (flags & rshflg)
							failed(com[0], restricted);
						else if ((a1 == 0 &&
						   (a1 = homenod.namval) == 0) ||
						    chdir(a1) < 0) 
							failed(a1, baddir);
						break;

					case SYSSHFT:
						if (dolc < 1)
							error(badshift);
						else {
							dolv++;
							dolc--;
						}
						assnum(&dolladr, dolc);
						break;

					case SYSWAIT:
						await(-1);
						break;

					case SYSREAD:
						exitval = readvar(&com[1]);
						break;

/*
					case SYSTST:
						exitval = testcmd(com);
						break;
 */

					case SYSSET:
						if (a1) {
							int argc = options(argn,
								com);

							if (argc > 1)
								setargs(com +
								    argn - argc);
						} else if (((COMPTR)t)->comset == 0)
							/*scan name chain & print*/
							namscan(printnam);
						break;

					case SYSRDONLY:
						exitval = N_RDONLY;
						/* FALLTHROUGH */
					case SYSXPORT:
						if (exitval == 0)
							exitval = N_EXPORT;

						if (a1)
							while (*++com)
								attrib(lookup(*com), exitval);
						else
							namscan(printflg);
						exitval = 0;
						break;

					case SYSEVAL:
						if (a1) {
							pth_ret xx;
							xx.str = com + 2;
							execexp(a1, xx);
						}
						break;

					case SYSUMASK:
						if (a1) {
							int i=0;
							unsigned char c;

							while ((c = (*a1++ -'0')) < 8)
								i = (i<<3)+c;
							umask(i);
						} else {
							int i, j;

							umask(i = umask(0));
							prc('0');
							for (j = 6; j >= 0; j-=3)
								prc(((i>>j)&7)+'0');
							newline();
						}
						break;

					default:
						internal = builtin(argn, com);

					}

					if (internal) {
						if (io) 
							error(illegal);
						chktrap();
						break;
					}
				} else if (t->treio == 0)
					break;
			}
			/* FALLTHROUGH */
		case TFORK:
			if (execflg && (treeflgs&(FAMP|FPOU)) == 0)
				parent = 0;
			else {
				while ((parent = fork()) == -1) {
					sleep(1);
				}
			}

			if (parent) {	/* This is the parent branch of fork; */
				/* it may or may not wait for the child. */
				if (treeflgs&FPRS && flags&ttyflg) {
					prn(parent);
					newline();
				}
				if (treeflgs & FPCL)
					closepipe(pf1);
				if ((treeflgs&(FAMP|FPOU)) == 0)
					await(parent);
				else if ((treeflgs&FAMP) == 0)
					post(parent);
				else
					assnum(&pcsadr, parent);

				chktrap();
				break;
			} else {
				/* this is the forked branch (child) of execute */
				flags |= forked;
				iotemp = 0;
				postclr();
				settmp();

				/* Turn off INTR and QUIT if `FINT'  */
				/* Reset ramaining signals to parent */
				/* except for those `lost' by trap   */
				oldsigs();
				if (treeflgs & FINT) {
					signal(SIGINT, SIG_IGN);
					signal(SIGQUIT, SIG_IGN);
				}

				/* pipe in or out */
				if (treeflgs & FPIN) {
					myrename(pf1[INPIPE], 0);
					close(pf1[OTPIPE]);
				}
				if (treeflgs & FPOU) {
					myrename(pf2[OTPIPE], 1);
					close(pf2[INPIPE]);
				}

				/* default std input for & */
				if (treeflgs&FINT && ioset == 0)
					myrename(chkopen(devnull, 0), 0);

				/* io redirection */
				initio(t->treio);
				if (type != TCOM)
					execute(((FORKPTR)t)->forktre, 1,
						NIL, NIL);
				else if (com[0] != ENDARGS) {
					setlist(((COMPTR)t)->comset, N_EXPORT);
					execa(com);
				}
				done();
			}

		case TPAR:
			nfd = dup(2);
			if (nfd < 0) 		        
				//error(duperr);
				error("xec dup error");
			myrename(nfd, output);
			execute(((PARPTR)t)->partre, execflg, NIL, NIL);
			done();

		case TFIL:
			{
				int pv[2];

				chkpipe(pv);
				if (execute(((LSTPTR)t)->lstlef, 0, pf1, pv) == 0)
					execute(((LSTPTR)t)->lstrit, execflg,
						pv, pf2);
				else
					closepipe(pv);
			}
			break;

		case TLST:
			execute(((LSTPTR)t)->lstlef, 0, NIL, NIL);
			execute(((LSTPTR)t)->lstrit, execflg, NIL, NIL);
			break;

		case TAND:
			if (execute(((LSTPTR)t)->lstlef, 0, NIL, NIL) == 0)
				execute(((LSTPTR)t)->lstrit, execflg, NIL, NIL);
			break;

		case TORF:
			if (execute(((LSTPTR)t)->lstlef, 0, NIL, NIL) != 0)
				execute(((LSTPTR)t)->lstrit, execflg, NIL, NIL);
			break;

		case TFOR:
			{
				NAMPTR n = lookup(((FORPTR)t)->fornam);
				STRING *args;
				DOLPTR argsav = 0;

				if (((FORPTR)t)->forlst == 0) {
					args = dolv + 1;
					argsav = useargs();
				} else {
					ARGPTR schain = gchain;

					gchain = 0;
					args = scan(getarg(((FORPTR)t)->forlst));
					trim(args[0]);
					gchain = schain;
				}
				loopcnt++;
				while (*args != ENDARGS && execbrk == 0) {
					assign(n, *args++);
					execute(((FORPTR)t)->fortre, 0, NIL, NIL);
					if (execbrk < 0)
						execbrk = 0;
				}
				if (breakcnt)
					breakcnt--;
				execbrk = breakcnt;
				loopcnt--;
				argfor = freeargs(argsav);
			}
			break;

		case TWH:
		case TUN:
			{
				int i = 0;

				loopcnt++;
				while (execbrk == 0 &&
				    (execute(((WHPTR)t)->whtre, 0, NIL, NIL) ==
				     0) == (type == TWH)) {
					i = execute(((WHPTR)t)->dotre, 0,
						NIL, NIL);
					if (execbrk < 0)
						execbrk = 0;
				}
				if (breakcnt)
					breakcnt--;
				execbrk = breakcnt;
				loopcnt--;
				exitval = i;
			}
			break;

		case TIF:
			if (execute(((IFPTR)t)->iftre, 0, NIL, NIL) == 0)
				execute(((IFPTR)t)->thtre, execflg, NIL, NIL);
			else
				execute(((IFPTR)t)->eltre, execflg, NIL, NIL);
			break;

		case TSW:
			{
				STRING r = mactrim(((SWPTR)t)->swarg);

				t = (TREPTR)(((SWPTR)t)->swlst);
				while (t) {
					ARGPTR rex = ((REGPTR)t)->regptr;

					while (rex) {
						STRING s;

						if (gmatch(r, s = macro(rex->argval)) ||
						    (trim(s), eq(r, s))) {
							execute(((REGPTR)t)->regcom,
								0, NIL, NIL);
							t = 0;
							break;
						} else
							rex = rex->argnxt;
					}
					if (t)
						t = (TREPTR)(((REGPTR)t)->regnxt);
				}
			}
			break;
		}
		exitset();
	}
	sigchk();
	tdystak(sav);
	return exitval;
}

void
execexp(STRING s, /*UFD*/ pth_ret f)
{
	FILEBLK fb;

	push(&fb);
	if (s) {
		estabf(s);
		fb.feval = f.str;
	} else if (f.fd >= 0)
		initf(f.fd);
	execute(cmd(NL, NLFLG|MTFLG), 0, NIL, NIL);
	pop();
}
