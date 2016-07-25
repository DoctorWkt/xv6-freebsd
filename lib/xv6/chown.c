#include <unistd.h>

/* xv6 has no chown(), but root can do anything */
int chown(const char *path, uid_t owner, gid_t group)
{
  return(0);
}
