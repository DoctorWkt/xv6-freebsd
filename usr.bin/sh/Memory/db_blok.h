#ifndef DEBUG
#define pr_sby()

#else

#define abort _exit
#define pr_sby()	prn(getpid()); prs(": stakbsy:\t"); \
	prn(staktop-stakbas); prs("/"); prn(staktop-stakbot); newline()

int 
chkbptr(struct blk *ptr)
{
	int	exf = 0;
	BLKPTR p = blokbas;
	BLKPTR q;
	int	us = 0, un = 0;

	for (;;)
	{
		q = BLK((Rcheat(p->word) & ~BUSY));

		if (p+1 == ptr)
			exf++;

		if (q < blokbas || q > bloktop)
			abort(3);

		if (p == bloktop)
			break;

		if (busy(p))
			us += q - p;
		else
			un += q - p;

		if (p >= q)
			abort(4);

		p = q;
	}
	if (exf == 0)
		abort(1);
	return 0;
}

int 
chkmem(void)
{
	BLKPTR p = blokbas;
	BLKPTR q;
	int	us = 0, un = 0;

	for (;;) {
		q = BLK((Rcheat(p->word) & ~BUSY));

		if (q < blokbas || q > bloktop)
			abort(3);

		if (p == bloktop)
			break;

		if (busy(p))
			us += q - p;
		else
			un += q - p;

		if (p >= q)
			abort(4);

		p = q;
	}

	prn(getpid());
	prs(": un/used/avail ");
	prn(un);
	blank();
	prn(us);
	blank();
	prn((BLK(bloktop) - BLK(blokbas)) - (un + us));
	newline();
	return 0;
}

size_t
blklen(char *q)
{
	BLKPTR pp = BLK(q);
	BLKPTR p;

	--pp;
	p = BLK((Rcheat(pp->word) & ~BUSY));

	return ((size_t)((long)p - (long)q));
}
#endif
