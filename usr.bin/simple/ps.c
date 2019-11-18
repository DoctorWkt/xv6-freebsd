#include <stdio.h>

/*
#include <xv6/types.h>
#include <xv6/stat.h>
#include <xv6/user.h>
*/
// From proc.h Proc structure
// Per-process state
enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
struct proc {
  enum procstate state; // Process state
  int pid;              // Process ID
  int ppid ;            // Parent process ID
  char name[16];        // Process name 
};

#define MAX_PROC 10

int
main(int argc, char *argv[]) {
  struct proc ptable[MAX_PROC];
  struct proc *p;
  int err;
  
  err = procs(10*sizeof(struct proc),&ptable);
  if(err !=0)
    printf("Error getting ptable");
  
  p = &ptable[0];
  printf(" PID  PPID  STATE     CMD\n");
  while(p != &ptable[MAX_PROC-1] && p->state != UNUSED){
       printf(" %4d %4d ",p->pid,p->ppid);
       switch(p->state){
       case UNUSED:
               printf(" %s ", "UNUSED  ");
               break;
       case EMBRYO:
               printf(" %s ", "EMBRYO  ");
               break;
       case SLEEPING:
               printf(" %s ", "SLEEPING");
               break;
       case RUNNABLE:
               printf(" %s ", "RUNNABLE");
               break;
       case RUNNING:
               printf(" %s ", "RUNNING ");
               break;
       case ZOMBIE:
               printf(" %s ", "ZOMBIE  ");
               break;
       } 
       printf(" %s\n", p->name);
       p++;
  }
         
  exit();
}

