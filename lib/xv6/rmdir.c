#include <unistd.h>

int rmdir(const char *path)
{
  return(unlink(path));
}
