#ifndef _UCC_TERMIOS_H
#define _UCC_TERMIOS_H

typedef unsigned short tcflag_t;
typedef unsigned char cc_t;
typedef unsigned int speed_t;

#define NCCS            20      /* size of cc_c array, some extra space
                                 * for extensions. */

/* 0x54 is just a magic number to make these relatively unique ('T') */
#define TCGETA          0x5405
#define TCSETA          0x5406

/* Primary terminal control structure. POSIX Table 7-1. */
struct termios {
  tcflag_t c_iflag;             /* input modes */
  tcflag_t c_oflag;             /* output modes */
  tcflag_t c_cflag;             /* control modes */
  tcflag_t c_lflag;             /* local modes */
  speed_t  c_ispeed;            /* input speed */
  speed_t  c_ospeed;            /* output speed */
  cc_t c_cc[NCCS];              /* control characters */
};

int tcgetattr(int,  struct termios *);
void cfmakeraw(struct termios *);
int tcsetattr(int, int, const struct termios *);

/* c_lflag bits */
#define ECHO            0x00000008  /* enable echoing of input characters */
#define ICANON          0x00000100  /* canonical input */

/* tcsetattr uses these */
#define TCSANOW            0        /* changes take effect immediately */

#endif
