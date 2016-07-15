#include <stdio.h>
#include <unistd.h>

int rename(const char *old, const char *new)
{
  int err;
  err= link(old, new);
  if (err==-1) return(-1);
  return(unlink(old));
}
