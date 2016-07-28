#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <stdio.h>

int getrusage(int who, struct rusage *usage)
{
  if (usage==NULL) { errno=EFAULT; return(-1); }

  // Dummy data
  usage->ru_utime.tv_sec=1;
  usage->ru_stime.tv_sec=1;
  usage->ru_maxrss=50;
  usage->ru_minflt=0;
  usage->ru_majflt=0;
  usage->ru_inblock=0;
  usage->ru_oublock=0;
  usage->ru_nvcsw=0;
  usage->ru_nivcsw=0;
  return(0);
}
