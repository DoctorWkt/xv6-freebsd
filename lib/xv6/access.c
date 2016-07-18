#include <unistd.h>

// Root can access anything
int access(const char *pathname, int mode)
{
  return(0);
}
