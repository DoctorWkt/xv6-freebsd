#ifdef SH_FASTSORT
#undef SH_TINYSORT
#endif
#define ex_ch(a,b) do { STRING tmp=*a; *a=*b; *b=tmp; } while(0)


#ifdef SH_TINYSORT
static void gsort(STRING l[], STRING r[]) {
	STRING *k, *L;
	if (l+1 >= r) return;
	k = l + (r-l)/2;
	ex_ch(l,k);
	for (L = l, k = l+1; k < r; k++)
		if (cf(*k, *l) < 0)
			{ ++L; ex_ch(L,k); }
	ex_ch(l,L);
	gsort(l,L);
	gsort(L+1,r);
}
#endif


#ifdef SH_FASTSORT
/* Quicksort with 3-way partitioning, ala Sedgewick */
/* Blame him for the scary variable names */
/* http://www.cs.princeton.edu/~rs/talks/QuicksortIsOptimal.pdf */
static void gsort(STRING l[], STRING r[]) {
	STRING *i, *p, *k, *j, *q, v;
	if (--r<=l) return;
	i=p = l-1;
	j=q = r;
	v = *r;
	for (;;) {
		while (++i != r && cf(*i,v)<0);
		while (cf(v,*--j)<0) if (j == l) break;
		if (i >= j) break;
		ex_ch(i,j);
		if (cf(*i,v)==0) { ++p; ex_ch(p,i); }
		if (cf(v,*j)==0) { --q; ex_ch(j,q); }
	}
	ex_ch(i,r); j = i-1; ++i;
	for (k=l;   k<p; k++, j--) { ex_ch(k,j); }
	for (k=r-1; k>q; k--, i++) { ex_ch(i,k); }
	gsort(l,j+1);
	gsort(i,r+1);
}
#endif
#undef ex_ch
