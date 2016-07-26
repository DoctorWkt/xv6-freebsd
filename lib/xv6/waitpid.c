#include <sys/types.h>
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *status, int options)
{
  pid_t p;

  while (1) {
    p=wait(status);
    if (p<0) return(p);
    if (p==pid) return(pid);
  }
}
