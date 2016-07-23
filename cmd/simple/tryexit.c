#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
  int status, exitvalue;
  pid_t wpid;
  pid_t pid=fork();

  switch(pid) {
    case -1: printf("fork error\n");
	     exit(1);

    case 0:  printf("In child, sleep then exit(5)\n");
	     sleep(3);
	     exit(5);

    default: printf("In parent, waiting for %d\n", pid);
	     wpid=wait(&status);
	     printf("Got back pid %d status %d\n", wpid, status);
	     if (WIFEXITED(status)) {
		exitvalue= WEXITSTATUS(status);
		printf("exit value was %d\n", exitvalue);
	     }
  }
  exit(0);
}
