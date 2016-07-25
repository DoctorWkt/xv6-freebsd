#include <sys/types.h>
#include <sys/stat.h>

/* xv6 has no FIFOs */
int mkfifo(const char *_path, mode_t _mode)
{
  return(-1);
}
