#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xv6/types.h>

#define stat xv6stat
#include <xv6/stat.h>
#undef stat

extern int _Fstat(int fd, struct xv6stat*);

/* The xv6 fstat() syscall returns a stat structure which isn't POSIX
 * compatible. This function does the syscall and returns a POSIX struct
 */
int fstat(int fildes, struct stat *buf)
{
  struct xv6stat s;
  int err;

  if (buf==NULL) return(-1);
  err= _Fstat(fildes, &s);
  if (err==-1) return(-1);

  buf->st_dev=   s.dev;
  buf->st_ino=   s.ino;
  buf->st_nlink= s.nlink;
  buf->st_uid=   0;
  buf->st_gid=   0;
  buf->st_rdev=  0;
  buf->st_size=  s.size;
  buf->st_atime= 0;
  buf->st_mtime= 0;
  buf->st_ctime= 0;

  buf->st_mode=  0777;		// Simulate a file acessible by all
  switch (s.type) {
    case T_DIR:   buf->st_mode |= S_IFDIR; break;
    case T_FILE:  buf->st_mode |= S_IFREG; break;
    case T_DEV:   buf->st_mode |= S_IFBLK; break;
  }
  return(0);
}
