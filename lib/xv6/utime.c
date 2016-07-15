#include <sys/types.h>
#include <utime.h>

/* Another missing syscall */
int utime(const char *_path, const struct utimbuf *_times)
{
  return(0);
}
