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
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return EINVAL;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
