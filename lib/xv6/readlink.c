#include <unistd.h>
#include <errno.h>

int readlink(const char *pathname, char *buf, int bufsiz)
{ errno=EINVAL; return(-1); }

int symlink(const char *target, const char *linkpath)
{ errno=EIO; return(-1); }
