#include <xv6/types.h>
#include <xv6/vfs.h>
#include <xv6/file.h>

int dev_zero_read(struct inode *ip, char *dst, int n)
{
  int i = 0;
  for (; i<n; ++i)
    dst[i] = 0;
  return n;
}

int dev_zero_write(struct inode *ip, char *buf, int n)
{
  return n;
}

void dev_zero_init(void)
{
  devsw[DEV_ZERO].write = dev_zero_write;
  devsw[DEV_ZERO].read = dev_zero_read;
}
