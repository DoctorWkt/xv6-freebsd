#include <sys/types.h>
#include <unistd.h>

int vfork(void)
{ return(fork()); }
