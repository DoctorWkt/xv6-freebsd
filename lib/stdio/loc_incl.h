/*
 * loc_incl.h - local include file for stdio library
 */
/* $Header: loc_incl.h,v 1.5 91/06/10 17:07:18 ceriel Exp $ */

#include	<stdio.h>

#define	io_testflag(p,x)	((p)->_flags & (x))

#include	<stdarg.h>

#ifdef _ANSI
int _doprnt(const char *format, va_list ap, FILE *stream);
int _doscan(FILE * stream, const char *format, va_list ap);
char *_i_compute(unsigned long val, int base, char *s, int nrdigits);
char *_f_print(va_list *ap, int flags, char *s, char c, int precision);
void __cleanup(void);

FILE *popen(const char *command, const char *type);
FILE *fdopen(int fd, const char *mode);

#ifndef	NOFLOAT
char *_ecvt(long double value, int ndigit, int *decpt, int *sign);
char *_fcvt(long double value, int ndigit, int *decpt, int *sign);
#endif	/* NOFLOAT */
#endif
char * _i_compute(unsigned long val, int base, char *s, int nrdigits);

#define	FL_LJUST	0x0001		/* left-justify field */
#define	FL_SIGN		0x0002		/* sign in signed conversions */
#define	FL_SPACE	0x0004		/* space in signed conversions */
#define	FL_ALT		0x0008		/* alternate form */
#define	FL_ZEROFILL	0x0010		/* fill with zero's */
#define	FL_SHORT	0x0020		/* optional h */
#define	FL_LONG		0x0040		/* optional l */
#define	FL_LONGDOUBLE	0x0080		/* optional L */
#define	FL_WIDTHSPEC	0x0100		/* field width is specified */
#define	FL_PRECSPEC	0x0200		/* precision is specified */
#define FL_SIGNEDCONV	0x0400		/* may contain a sign */
#define	FL_NOASSIGN	0x0800		/* do not assign (in scanf) */
#define	FL_NOMORE	0x1000		/* all flags collected */
