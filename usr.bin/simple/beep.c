#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

int main(int argc, char** argv)
{
  int frq;
  
  if (argc == 2) {
    frq = atoi(argv[1]);
  } else {
    frq = 1000;
  }
    
  beep(frq);
  sleep(1);
  beep(0);
  
  return 0;
}
