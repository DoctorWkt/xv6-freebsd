int tgetent(char *bp, char *name);
int tfindent(char *bp, char *name);
int tnchktc(void);
int tnamatch(char *np);
int tgetnum(int id);
int tgetflag(int id);
char *tgetstr(char *id, char **area);
char *tgoto(char *CM, int destcol, int destline);
int tputs(char *cp, int affcnt, int (*outc)());
