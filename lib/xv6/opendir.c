/* Functions to read from a directory. 		Author: Warren Toomey */

#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>

int closedir(DIR *dirp)
{
  if (dirp==NULL) return(-1);
  return(close(dirp->dd_fd));
}

DIR *opendir(const char *dirname)
{
  int dd_fd;
  DIR *dirp;

  // TODO: Add in a stat() to ensure that we are opening a directory

  if (dirname==NULL) return(NULL);
  dd_fd= open(dirname, O_RDONLY);
  if (dd_fd==-1) return(NULL);
  dirp= (DIR *)malloc(sizeof(DIR));
  if (dirp==NULL) return(NULL);
  dirp->dd_fd= dd_fd;
  return(dirp);
}

struct dirent *readdir(DIR *dirp)
{
  struct dirent *d;
  int err;

  if (dirp==NULL) return(NULL);
  d= (struct dirent *)malloc(sizeof(struct dirent));
  if (d==NULL) return(NULL);
  err= read(dirp->dd_fd, d, sizeof(struct dirent));
  if (err < sizeof(struct dirent)) return(NULL);
  return(d);
}

void rewinddir(DIR *dirp)
{
  lseek(dirp->dd_fd, 0, SEEK_SET);
}
