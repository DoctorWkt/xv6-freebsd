#include <unistd.h>

/* xv6 has no chown(), but root can do anything */
int chown(const char *path, Uid_t owner, Gid_t group)
{
  return(0);
}
