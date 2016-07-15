#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/* xv6 has no stat(), so we simulate it with fstat() */
int stat(const char *path, struct stat *buf)
{
  int fd, err;
  fd= open(path, O_RDONLY);
  if (fd==-1) return(-1);
  err= fstat(fd, buf);
  close(fd);
  return(err);
}
