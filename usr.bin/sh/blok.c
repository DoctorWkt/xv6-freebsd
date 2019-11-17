/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#define	BUSY 01
#define	busy(x)	(Rcheat((x)->word) & BUSY)
#include "db_blok.h"

/*
 *	storage allocator
 *	(circular first fit strategy)
 */

BLKPTR	stakbsy=0;
BLKPTR	blokbas, bloktop;	/* top of arena (last blok) */
static BLKPTR blokp;		/* current search pointer */

void *
shalloc(size_t nbytes)
{
	unsigned rbytes = align(size_t,(nbytes+BYTESPERWORD),BYTESPERWORD);
	for (;;)
	{
		int	c = 0;
		BLKPTR	q, p = blokp;
		do {
			if (!busy(p)) {
				while (!busy(q = p->word))
					p->word = q->word;
				if (ADR(q) >= ADR(p) + rbytes) {
					blokp = BLK(ADR(p) + rbytes);
					if (q > blokp)
						blokp->word = p->word;
					p->word = BLK((Rcheat(blokp)|BUSY));
					return (ADR(p + 1));
				}
			}
			q = p;
			p = BLK((Rcheat(p->word) & ~BUSY));
		} while (p > q || (c++) == 0);
		addblok(rbytes);
	}
}

void
addblok(unsigned reqd)
{
	char *oldstak = stakbot;
	unsigned stlen = relstak();
	if (stakbas != staktop) {
		BLKPTR blokstak = BLK(stakbas) - 1;
		blokstak->word = stakbsy;
		stakbsy = blokstak;
		pr_sby();

		endstak();
		bloktop->word = BLK((Rcheat(staktop) | BUSY));
		bloktop = BLK(staktop);
	}

	blokp = bloktop->word = BLK((ADR(bloktop) + 256));
	bloktop = bloktop->word;

	stakbas = stakbot = ADR(bloktop + 2);
	stakbot = getstak(stlen);
	staktop = stlen ? movstr(oldstak, stakbot) : stakbot;

	bloktop->word = BLK((Rcheat(blokbas) | BUSY));
}

void
shfree(void *ap)
{
	BLKPTR p;

	if ((p = BLK(ap)) && p < bloktop && p > blokbas) {
#ifdef DEBUG
		chkbptr(p); chkmem();
#endif
		--p;
		p->word = BLK((Rcheat(p->word) & ~BUSY));
	}
}
