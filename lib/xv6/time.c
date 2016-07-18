#include <time.h>

extern int _Time(void);

time_t time(time_t *timeptr)
{
  time_t x= _Time();
  if (timeptr) *timeptr=x;
  return(x);
}
