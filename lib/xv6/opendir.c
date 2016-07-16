/* Functions to read from a directory. 		Author: Warren Toomey */

#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>

int closedir(DIR *dirp)
{
  if (dirp==NULL) return(-1);
  return(close(dirp->fd));
}

DIR *opendir(const char *dirname)
{
  int fd;
  DIR *dirp;

  // TODO: Add in a stat() to ensure that we are opening a directory

  if (dirname==NULL) return(NULL);
  fd= open(dirname, O_RDONLY);
  if (fd==-1) return(NULL);
  dirp= (DIR *)malloc(sizeof(DIR));
  if (dirp==NULL) return(NULL);
  dirp->fd= fd;
  return(dirp);
}

struct dirent *readdir(DIR *dirp)
{
  struct dirent *d;
  int err;

  if (dirp==NULL) return(NULL);
  d= (struct dirent *)malloc(sizeof(struct dirent));
  if (d==NULL) return(NULL);
  err= read(dirp->fd, d, sizeof(struct dirent));
  if (err < sizeof(struct dirent)) return(NULL);
  return(d);
}
