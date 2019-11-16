#include <xv6/types.h>
#include <xv6/x86.h>
#include <xv6/defs.h>
#include <xv6/date.h>
#include <xv6/param.h>
#include <xv6/memlayout.h>
#include <xv6/mmu.h>
#include <xv6/proc.h>
#include <xv6/syscall.h>

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  int exitvalue=0;
  argint(0, &exitvalue);
  exit(exitvalue);
  return 0;  // not reached
}

int
sys_wait(void)
{
  int *statusptr;
  if(argptr(0, (void *)&statusptr, 4) < 0)
    statusptr=(int *)0;
  return wait(statusptr);
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return EINVAL;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return EINVAL;
  addr = proc->sz;
  if(growproc(n) < 0)
    return ENOMEM;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return EINVAL;
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      return EINVAL;
    }
    sleep(&ticks, (struct spinlock *)0);
  }
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  xticks = ticks; // atomic read
  return xticks;
}

// shutdown QEMU
int
sys_halt(void)
{
  do_shutdown();  // never returns
  return 0;
}
