#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// Root can access anything as long as it exists
int access(const char *pathname, int mode)
{
  int fd= open(pathname, mode);
  close(fd);
  return(fd);
}
