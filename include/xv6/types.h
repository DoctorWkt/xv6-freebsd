typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;

#define TPS 1000   // ticks-per-second
#define SCHED_INTERVAL (TPS/100)  // See kern/trap.c
