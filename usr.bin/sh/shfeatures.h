
/* define next if you want '^' == '|'   hat sing is equivalent to pipe */
// #define WANT_HAT_AS_PIPE

/* define next to avoid -Wall compiler warnings */
// #define WANT_INT_IN_CTYPE_H

/* if defined shell check for incoming mail */
// #define SH_MAIL

/* start with: setuid(getuid()); setgid(getgid()); */
#define SH_SETUIDGID

/* if the name contain the letter r the shell is restricted */
// #define SH_RESTRICTED

/* small sort routines for dirlist sorting */
// #define SH_TINYSORT

/* fast sort routines for dirlist sorting */
// #define SH_FASTSORT

/* these are used in mmapalloc.c sbrkalloc.c */
// #define WANT_CALLOC
// #define WANT_REALLOC

/* used in sbrkalloc.c;  Sh will have an static buffer for readdir()
 * this is for systems which uses malloc for opendir/readdir */
// #define STATIC_READDIR_BUFFER   4096+256

/* define next if readdir() is broken und return names which are
 * _not_ zero terminated */
// #define WANT_STRNCPY_readdir_name

/* define next if you want pushstak macro */
// #define WANT_PUSHSTAK_MACRO

/* change MAXTRAP;  default is 17 */
// #define WANT_MAXTRAP 32
// #define WANT_MAXTRAP 64
