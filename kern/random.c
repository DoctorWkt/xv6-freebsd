#include <xv6/types.h>
#include <xv6/vfs.h>
#include <xv6/file.h>

int dev_random_read(struct inode *ip, char *dst, int n)
{
  int i = 0;
  for (; i<n; ++i) {
    dst[i] = 42;
  }
  return n;
}

int dev_random_write(struct inode *ip, char *buf, int n)
{
  return n;
}

void dev_random_init(void)
{
  devsw[DEV_RANDOM].write = dev_random_write;
  devsw[DEV_RANDOM].read = dev_random_read;
}
