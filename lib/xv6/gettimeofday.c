#include <sys/time.h>
#include <errno.h>

extern int _Time(void);

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  if (tv==NULL) { errno=EFAULT; return(-1); }
  tv->tv_sec= _Time();
  tv->tv_usec= 0; return(0); 
}

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{ return(0); }
