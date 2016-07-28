/*
 *	UNIX shell
 *
 *	Bell Telephone Laboratories
 */

/*
 * To use stack as temporary workspace across
 * possible storage allocation (eg name lookup)
 * a) get ptr from `relstak'
 * b) can now use `pushstak'
 * c) then reset with `setstak'
 * d) `absstak' gives real address if needed
 */

extern void pushstak(char c);

#define relstak()	(staktop-stakbot)	/* offset into stack */
#define zerostak()	(*staktop=0)		/* zero top byte */
#define setstak(x)	(staktop=absstak(x))	/* pop back to start */
#define absstak(x)	(stakbot+Rcheat(x))	/* address from offset */

/* uses sbr to allocate RAM */
extern char *setbrk(int incr);

/* For use after `locstak' to hand back
 * new stack top and then allocate item
 */
extern char *endstak(void);

/* Copy a string onto the stack and
 * allocate the space.
 */
extern char *cpystak(char *);

/* Append a string onto the stack
 */
extern char *appstak(char *x);

/* Allocate given ammount of stack space */
extern char *getstak(int);

void	addblok(unsigned reqd);
void	stakchk(void);
void	tdystak(char *sav /* , struct ionod *iosav */);

extern char *staktop;	/* Top of current item */
extern char *stakbot;	/* Base of current item */
extern char *stakend;	/* Top of entire stack; new, geoff */
extern char *stakbas;	/* Base of entire stack */

/* A chain of ptrs of stack blocks that
 * have become covered by heap allocation.
 * `tdystak' will return them to the heap.
 */
extern BLKPTR	stakbsy;
extern BLKPTR	bloktop;
extern BLKPTR   blokbas;

#define BRKINCR 1024
#define BRKMAX  2048
#define SBRK_offset		(12*BYTESPERWORD)
