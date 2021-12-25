// #define DEBUG

#ifdef DEBUG 
#define pr_info()	info(incr)
#else 
#define pr_info()
#endif

#ifdef DEBUG
#define myP(s,a,b) prs(s); prn((size_t)(a) - (size_t)(b))

static info(int incr) {
	prn(getpid());
	prs(": incr:\t");
	if (incr<0) { prs("-"); incr = -incr; }
	prn(incr);

	myP("\nBLtop-BLbas:\t", bloktop, blokbas);
	myP("\nSTtop-STbas:\t", staktop, stakbas);
	myP("\nSTend-STtop:\t", stakend, staktop);
	newline();
}

#undef myP
#endif
