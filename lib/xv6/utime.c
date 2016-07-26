#include <sys/types.h>
#include <utime.h>
#include <sys/time.h>

/* Another missing syscall */
int utime(const char *_path, const struct utimbuf *_times)
{ return(0); }

int utimes(const char *filename, const struct timeval times[2])
{ return(0); }
