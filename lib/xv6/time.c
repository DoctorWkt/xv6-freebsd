#include <time.h>

/* xv6 has no time concept! */
time_t time(time_t *_timeptr)
{
  return(0);
}
