#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
  int x;

  while (1) {
    x= time(NULL);
    printf("x is %d\n",x);
    sleep(1);
  }
  exit(0);
}
