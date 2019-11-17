/* Functions to read from a directory. 		Author: Warren Toomey */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#define dirent xv6dirent
#include <xv6/types.h>
#include <xv6/vfs.h>
#undef dirent

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
  struct xv6dirent x;
  int err;

  if (dirp==NULL) return(NULL);
  d= (struct dirent *)malloc(sizeof(struct dirent));
  if (d==NULL) return(NULL);
  err= read(dirp->dd_fd, &x, sizeof(struct xv6dirent));
  if (err < sizeof(struct xv6dirent)) return(NULL);

  // Copy over into BSD struct
  strncpy(d->d_name, x.name, DIRSIZ);
  d->d_namlen= strlen(d->d_name);
  d->d_fileno= x.inum;
  return(d);
}

void rewinddir(DIR *dirp)
{
  lseek(dirp->dd_fd, 0, SEEK_SET);
}
