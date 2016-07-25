#include <limits.h>

int getpagesize(void)
{ return(4096); }

int getdtablesize(void)
{ return(OPEN_MAX); }
