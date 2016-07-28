/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 */

#include "defs.h"
#include <dirent.h>

/* globals (file name generation)
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches character class
 * "[...a-z...]" in params matches a through z.
 *
 */

static void addg(STRING as1, STRING as2, STRING as3);

static DIR *
openqdir(char *name)
{
	char *s;
	char buf[MAXNAMLEN+1];
	unsigned int len = 0;

	/* strip 0200 quoting bits */

	for (s=buf; *name && len < sizeof(buf)-1; len++, s++, name++)
		*s = (*name & STRIP); 
	*s = 0;
	/* now do normal opendir */
	return opendir(buf);
}

int
expand(STRING as, int rflg)
{
	int count;
	BOOL	dir = 0;
	STRING rescan = 0, s, cs;
	ARGPTR	schain = gchain;
	DIR	*dirf;

	if (trapnote & SIGSET)
		return(0);

	s = cs = as;

	/* check for meta chars */
	{
		BOOL slash = 0;

		while (!fngchar(*cs))
			if (*cs++ == 0)
				if (rflg && slash)
					break;
				else
					return(0);
			else if (*cs == '/')
				slash++;
	}

	for (; ;) {
		if (cs == s) {
			s = nullstr;
			break;
		} else if (*--cs == '/') {
			*cs = 0;
			if (s == cs)
				s = "/";
			break;
		}
	}
	if ((dirf = openqdir(*s? s: ".")) != NIL)
		dir = TRUE;
	count = 0;
	if (*cs == 0)
		*cs++ = 0200;
	if (dir) {		/* check for rescan */
		STRING rs = cs;
		struct dirent *e;

		do {
			if (*rs == '/') {
				rescan = rs;
				*rs = 0;
				gchain = 0;
			}
		} while (*rs++);

		while ((e = readdir(dirf)) && (trapnote & SIGSET) == 0) {
#ifdef WANT_STRNCPY_readdir_name
			char entry[MAXNAMLEN+1];
			strncpy(entry, e->d_name, sizeof entry - 1);
			entry[sizeof entry - 1] = 0;
#else
			char *entry = e->d_name; /* zero terminated? */
#endif
			if ((entry[0] == '.' && *cs != '.'))
				if (entry[1] == 0 ||
				    (entry[1] == '.' && entry[2] == 0))
					continue;
			if (gmatch(entry, cs)) {
				addg(s, entry, rescan);
				count++;
			}
		}
		closedir(dirf);

		if (rescan) {
			ARGPTR rchain = gchain;

			gchain = schain;
			if (count) {
				count = 0;
				for (; rchain; rchain = rchain->argnxt)
					count += expand(rchain->argval, 1);
			}
			*rescan = '/';
		}
	}

	{
		char c;

		for (s = as; (c = *s); )
			*s++ = (c & STRIP? c: '/');
	}
	return count;
}

int
gmatch(STRING s, STRING p)
{
	int scc;
	char c;

	if ((scc = *s++))
		if ((scc &= STRIP) == 0)
			scc = 0200;
	switch (c = *p++) {
	case '[':
		{
			BOOL ok = 0;
			int lc = 077777;

			while ((c = *p++)) {
				if (c == ']')
					return(ok? gmatch(s, p): 0);
				else if (c == MINUS) {
					if (lc <= scc && scc <= *p++)
						ok++;
				} else
					if (scc == (lc = (c & STRIP)))
						ok++;
			}
			return 0;
		}

	default:
		if ((c & STRIP) != scc)
			return 0;

	case '?':
		return scc? gmatch(s, p): 0;

	case '*':
		if (*p == 0)
			return 1;
		--s;
		while (*s)
			if (gmatch(s++, p))
				return 1;
		return 0;

	case 0:
		return scc == 0;
	}
}

static void
addg(STRING as1, STRING as2, STRING as3)
{
	STRING	s1, argp = getstak(BYTESPERWORD);
	int c;

	s1 = as1;
	while ((c = *s1++)) {
		if ((c &= STRIP) == 0) {
			pushstak('/');
			break;
		}
		pushstak(c);
	}
	s1 = appstak(as2);
	if ((s1 = as3)) {
		pushstak('/');
		appstak(++s1);
	}

	endstak();
	makearg(argp);
}

void
makearg(STRING args)
{
	((ARGPTR)args)->argnxt = gchain;
	gchain = (ARGPTR)args;
}
