/* roff - text justifier		Author: George L. Sicherman */

/*
 *	roff - C version.
 *	the Colonel.  19 May 1983.
 *
 *	Copyright 1983 by G. L. Sicherman.
 *	You may use and alter this software freely for noncommercial ends
 *	so long as you leave this message alone.
 *
 *	Fix by Tim Maroney, 31 Dec 1984.
 *	.hc implemented, 8 Feb 1985.
 *	Fix to hyphenating with underlining, 12 Feb 1985.
 *	Fixes to long-line hang and .bp by Dave Tutelman, 30 Mar 1985.
 *	Fix to centering valve with long input lines, 4 May 1987.
 *
 *	Functionality removed and converted to a simpler C dialect so that
 *	it might be runnable on PDP-7 Unix - Warren Toomey, April 2016.
 */

#ifdef H_LANG
#define EOF 4
int strlen(char *s);
void strcpy(char *dst, char *src);
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
void printn(int n);
#endif

#define MAXLENGTH 70
#define MAXTITLE 70
int UNDERL=0200;

char tempbuf[10];       /* Used by itoa() */
int sflag; int hflag;
char holdword[MAXLENGTH];
char *holdp;
char assyline[MAXLENGTH];
int assylen;
char ohead[MAXTITLE]; char ofoot[MAXTITLE];
int depth;
int adjtoggle;
int isrequest = 0;
int o_cc = '.';			/* CONTROL CHARACTER */
int o_tc = ' ';			/* TABULATION CHARACTER */
int o_in = 0;			/* INDENT SIZE */
int o_ta[] = {
	9, 17, 25, 33, 41, 49, 57, 65, 73, 81, 89, 97, 105,
	113, 121, 129, 137, 145, 153, 161};
int n_ta = 20;			/* #TAB STOPS */
int o_ll = 65; int o_ad = 1;
int o_pl = 66;
int o_sp = 0;
int o_m1 = 2; int o_m2 = 2; int o_m3 = 1; int o_m4 = 3;
int o_ul = 0;
int page_no = -1;
int line_no = 9999;
int n_outwords;
int TXTLEN;
char c;				/* LAST CHAR READ */
char cachec=0;			/* One char cache used by suck() */

int col_no=0;			/* Used by spit() */
int n_blanks=0;			/* Used by spit() */

FILE *infile;

char *itoa(int n);
char *pgform();
int isalnum(int c);
int isdigit(int c);
int islegal(int c);
int isspace(int c);
int readline();
int readreq();
int skipsp();
int suck();
int titlen(char *t, char c, int k);
int tread(char *s);
void beginpage();
void blankline();
void bumpword();
void do_ta();
void endpage();
void newpage();
int nread();
void readfile();
void spit(char c);
void spits(char *s);
void tabulate();
void writebreak();
void writeline(int adflag, int flushflag);
void writetitle(char *t);

int main(int argc, char *argv[])
{
  if (argc != 2) {
    write(2, "Usage: roff file\n", 17); exit(1);
  }
  infile= fopen(argv[1], "r");
  if (infile==NULL) {
    write(2, "Unable to open file\n", 20); exit(1);
  }
  

  TXTLEN = o_pl-o_m1-o_m2-o_m3-o_m4 - 2;
  assylen = 0;
  assyline[0] = 0;
  readfile();

  writebreak();
  endpage();
#ifdef H_LANG
  exit();
#else
  exit(0);
#endif
}


void readfile()
{
  while (readline()) {
	if (isrequest) continue;
  }
}

int readline()
{
  int startline; int doingword;
  isrequest = 0;
  startline = 1;
  doingword = 0;
  c = suck();
  if (c == '\n') {
	o_sp = 1;
	writebreak();
	goto out;
  } else if (isspace(c))
	writebreak();
  while(1) {
	if (c == EOF) {
		if (doingword) bumpword();
		break;
	}
	if (isspace(c)) {
	    if (doingword == 0) {
		startline = 0;
		if (c==' ') {
			assyline[assylen] = ' ';
			assylen++;
		}
		if (c=='\t') {
		    	tabulate();
		}
		if (c=='\n')	goto out;
		c = suck();
		continue;
	    }
	}
	if (isspace(c)) {
	    if (doingword) {
		bumpword();
		if (c == '\t')
			tabulate();
		else if (assylen) {
			assyline[assylen] = ' ';
			assylen++;
		}
		doingword = 0;
		if (c == '\n') break;
	    }
	}
	if (isspace(c) == 0) {
		if (doingword) {
			if (o_ul)
				*holdp = c | UNDERL;
			else
				*holdp = c;
			holdp++;
			goto readline1;
		}
		if (startline) {
		    if (c == o_cc) {
			    isrequest = 1;
			    return(readreq());
		    }
		} 
		doingword = 1;
		holdp = holdword;
		if (o_ul)
			*holdp = c | UNDERL;
		else
			*holdp = c;
		holdp++;
	}
readline1:
	startline = 0;
	c = suck();
  }
out:
  if (o_ul) o_ul--;
  if (c != EOF)
    return(1);
  else
    return(0);
}

/*
 *	bumpword - add word to current line.
 */

void bumpword()
{
  char *x;
  *holdp = '\0';
  if (assylen + strlen(holdword) > o_ll - o_in) {
	writeline(o_ad, 0);
  }
  x= assyline+assylen;
  strcpy(x, holdword);
  assylen = assylen + strlen(holdword);
  holdp = holdword;
}

void tabulate()
{
  int j;
  for (j = 0; j < n_ta; j++)
	if (o_ta[j] - 1 > assylen + o_in) {
		while (assylen + o_in < o_ta[j] - 1) {
			assyline[assylen] = o_tc;
			assylen++;
		}
		return;
	}

  /* NO TAB STOPS REMAIN */
  assyline[assylen] = o_tc; assylen++;
}

int readreq()
{
  int r;
  if (skipsp()) goto readend;
  c = suck();
  if (c == EOF) goto readend;
  if (c == '\n') goto readend;
  if (c == '.') {
	while (1) {
	    c = suck();
	   if (c == EOF) break;
	   if (c == '\n') break;
	}
	goto readend;
  }
  r = c << 9;
  c = suck();
  if (c != EOF) { 
     if (c != '\n') {
	r = r | c;
     }
  }

  if (r==0157150) c = tread(ohead);	/* oh */
  if (r==0163160) {			/* sp */
	o_sp=nread();
	writebreak();
  }
  if (r==0164141) do_ta();		/* ta */
  if (r==0165154) o_ul=nread();		/* ul */
  
  while (1) {
    if (c == EOF) break;
    if (c == '\n') break;
    c= suck();
  }
readend:
  if (c != EOF)
    return(1);
  else
    return(0);
}

int tread(char *s)	// OK
{
  int leadbl=0;
  while(1) {
	c = suck();
	if (c == ' ')
	    if (leadbl==0) continue;
	if (c == EOF) {
		*s = '\0';
		return(c);
	}
 	if (c == '\n') {
		*s = '\0';
		return(c);
	}
	*s = c; s++;
	leadbl++;
  }
}

int nread()		// OK
{
  int i=0;
  int f;
  f = 0;
  if (skipsp()==0)
        while(1) {
		c = suck();
		if (c == EOF) break;
		if (isspace(c)) break;
		if (isdigit(c)) {
			f++;
			i = i * 10 + c - '0';
		} else
			break;
	}
  if (f==0)
      i = 1;
  return(i);
}




void do_ta()
{
  int v;
  n_ta = 0;
  while(1) {
	v=nread(v);
	if (v == 1)
		return;
	else {
		o_ta[n_ta] = v; n_ta++;
	}
	if (c == '\n') break;
	if (c == EOF) break;
  }
}


int skipsp()
{
  while(1) {
	c= suck();
	if (c==EOF) 	return(1);
	if (c=='\n') 	return(1);
	if (c==' ') 	continue;
	if (c=='\t')	continue;
	cachec= c;		/* Unget the character */
	return(0);
  }
}

void writebreak()
{
  int q;
  if (assylen) writeline(0, 1);
  q = TXTLEN;
  if (o_sp) {
	if (o_sp + line_no > q)
		newpage();
	else if (line_no)
		while (o_sp) {
		    blankline(); o_sp--;
		}
  }
}

void blankline()
{
  if (line_no >= TXTLEN) newpage();
  spit('\n');
  line_no++;
}

void writeline(int adflag, int flushflag)
{
  int j; int q;
  for (j = assylen - 1; j; j--) {
	if (assyline[j] == ' ')
		assylen--;
	else
		break;
  }
  q = TXTLEN;
  if (line_no >= q) newpage();
  for (j = 0; j < assylen; j++) spit(assyline[j]);
  spit('\n');
  assylen = 0;
  assyline[0] = 0;
  line_no++;
  if (flushflag==0) {
	strcpy(assyline, holdword);
	assylen = strlen(holdword);
	*holdword = 0;
	holdp = holdword;
  }
}


void newpage()
{
  if (page_no >= 0)
	endpage();
  else
	page_no = 1;
  beginpage();
}

void beginpage()
{
  int i;
  for (i = 0; i < o_m1; i++) spit('\n');
  writetitle(ohead);
  for (i = 0; i < o_m2; i++) spit('\n');
  line_no = 0;
}

void endpage()
{
  int i;
  for (i = line_no; i < TXTLEN; i++) {
      spit('\n');
      line_no++;
  }
  for (i = 0; i < o_m3; i++) spit('\n');
  writetitle(ofoot);
  for (i = 0; i < o_m4; i++) spit('\n');
  page_no++;
}


void writetitle(char *t)
{
  char d; char *pst;
  int j; int l; int m; int n;
  int pstlen;

  d = *t;
  if (d=='\0') {
	spit('\n');
	return;
  }
  pst = pgform();
  pstlen= strlen(pst);
  t++;
  l = titlen(t, d, pstlen);
  while (1) {
    if (*t == 0) break;
    if (*t == d) break;
	if (*t == '%')
		spits(pst);
	else
		spit(*t);
	t++;
  }
  if (*t == '\0') {
	spit('\n');
	return;
  }
  t++;
  m = titlen(t, d, pstlen);
  for (j = l; j < (o_ll - m) / 2; j++) spit(' ');
  while (1) {
    if (*t == 0) break;
    if (*t == d) break;
	if (*t == '%')
		spits(pst);
	else
		spit(*t);
	t++;
  }
  if (*t == '\0') {
	spit('\n');
	return;
  }
  if ((o_ll - m) / 2 > l)
	m = m + (o_ll - m) / 2;
  else
	m = m + l;
  t++;
  n = titlen(t, d, pstlen);
  for (j = m; j < o_ll - n; j++) spit(' ');
  while (1) {
    if (*t == 0) break;
    if (*t == d) break;
	if (*t == '%')
		spits(pst);
	else
		spit(*t);
	t++;
  }
  spit('\n');
}

char *pgform()
{
  char *pst;
  pst= itoa(page_no);
  return(pst);
}

int titlen(char *t, char c, int k)
{
  int q;
  q = 0;
  while (1) {
        if (*t == 0) break;
        if (*t == c) break;
	if (*t == '%')
        	q= q + k;
      	else
        	q++;
      	t++;
  }
  return(q);
}

void spits(char *s)
{
  while (*s) {
	spit(*s); s++;
  }
}

void spit(char c)
{
  int ulflag;
  ulflag = c & UNDERL;
  c = c & (UNDERL ^ -1); 			// c = c & ~UNDERL;
	
  if (c != ' ') {
    if (c != '\n') {
      if (n_blanks) {
	while (n_blanks) {
		putchar(' ');
		col_no++;
		n_blanks--;
	}
      }
    }
  }
  if (ulflag) {
     if (isalnum(c)) {
	putchar('_'); putchar(010);	/* _\b */
     }
  }
  if (c == ' ')
	n_blanks++;
  else {
	putchar(c);
	col_no++;
  }
  if (c == '\n') {
	col_no = 0;
	n_blanks = 0;
  }
}

int suck()		// OK
{
  while(1) {
	c = cachec; cachec=0;	/* Get the cached char first */
	if (c=='\0') c = fgetc(infile);
	if (islegal(c)) return(c);
  }
}

int isspace(int c)
{
    if (c == ' ') return(1);
    if (c == '\t') return(1);
    if (c == '\n') return(1);
    return(0);
}


int isalnum(int c)	// OK
{
    if (c >= 'A') {
        if (c <= 'Z') {
            return(1);
        }
    }
    if (c >= 'a') {
        if (c <= 'z') {
            return(1);
        }
    }
    if (c >= '0') {
        if (c <= '9') {
            return(1);
        }
    }
    return(0);
}

int isdigit(int c)	// OK
{
    if (c >= '0') {
        if (c <= '9') {
            return(1);
        }
    }
    return(0);
}

int islegal(int c)		// OK
{
    if (c >= ' ') {
        if (c <= '~') {
            return(1);
        }
    }
    if (c == ' ') return(1);
    if (c == '\t') return(1);
    if (c == '\n') return(1);
    if (c == EOF) return(1);
    return(0);
}

/* Given a number, convert the number
 * into ASCII digits and store in the
 * printbuf array. Null terminate the
 * string. Return a pointer to the first
 * digit in the character.
 */
char *itoa(int n)		// OK
{
  char *digitptr;
  digitptr= tempbuf + 9;        /* i.e digiptr= &tempbuf[9] */
  *digitptr= '\0';
  digitptr--;

  while (n>0) {
    *digitptr = (n%10) + '0';   /* Store a digit */
    digitptr--;
    n=n/10;
  }

  return(digitptr+1);
}

#ifdef H_LANG
void strcpy(char *dst, char *src)	// OK
{
  while (*src) {
    *dst= *src;
    dst++; src++;
  }
  *dst='\0';
}

int strlen(char *s)	// OK
{
  int n=0;
  while (*s) {
    s++; n++;
  }
  return(n);
}
#else
void printn(int n)
{
  printf("%06o\n", n);
}
#endif
