#include <sys/types.h>
#include <sys/stat.h>

/* xv6 has no chmod(), but root can do anything */
int chmod(const char *path, mode_t mode)
{ return(0); }

int fchmod(int fd, mode_t mode)
{ return(0); }
