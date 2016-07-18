#include <unistd.h>

extern int _Sleep(int ticks);

unsigned int sleep(unsigned int seconds)
{
  // Convert seconds into ticks
  seconds *= 100;
  return(_Sleep(seconds));
}
