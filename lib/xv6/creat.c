#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int creat(const char *path, Mode_t mode)
{
  return( open(path, mode, O_CREAT|O_WRONLY|O_TRUNC) );
}
