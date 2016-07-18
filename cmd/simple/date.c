/* date: print the current date & time		Author: Warren Toomey */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
  time_t tim= time(NULL);
  printf("%s",ctime(&tim));
  exit(0);
}
