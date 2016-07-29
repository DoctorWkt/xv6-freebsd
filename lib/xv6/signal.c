#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>


// xv6 has no signals

__sighandler_t signal(int sig, __sighandler_t func)
{
 return(SIG_DFL);
}

#if 0
void longjmp(jmp_buf env, int val)
{
  return;
}

int setjmp(jmp_buf env)
{
  return(0);
}
#endif

int sigblock(int mask) { return(0); }

int sigsetmask(int mask) { return(0); }

int siggetmask(void) { return(0); }
