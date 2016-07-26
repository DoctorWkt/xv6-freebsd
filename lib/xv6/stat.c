#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
extern int errno;

/* xv6 has no stat(), so we simulate it with fstat() */
int stat(const char *path, struct stat *buf)
{
  int fd, err;
  errno=0;
  fd= open(path, O_RDONLY);
  if (fd==-1) { errno= ENOENT; return(-1); }
  err= fstat(fd, buf);
  close(fd);
  return(err);
}

int lstat(const char *path, struct stat *buf)
{
  return(stat(path,buf));
}
