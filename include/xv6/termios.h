#ifndef _TERMIOS_H_
#define _TERMIOS_H_

#define ECHO            0x00000008      /* enable echoing */
#define ICANON          0x00000100      /* canonicalize input lines */

/* 0x54 is just a magic number to make these relatively unique ('T') */
#define TCGETA          0x5405
#define TCSETA          0x5406

#define NCCS            20

typedef unsigned long   tcflag_t;
typedef unsigned char   cc_t;
typedef long            speed_t;

struct termios {
        tcflag_t        c_iflag;        /* input flags */
        tcflag_t        c_oflag;        /* output flags */
        tcflag_t        c_cflag;        /* control flags */
        tcflag_t        c_lflag;        /* local flags */
        cc_t            c_cc[NCCS];     /* control chars */
        long            c_ispeed;       /* input speed */
        long            c_ospeed;       /* output speed */
};

#endif
