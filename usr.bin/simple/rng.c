#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char** argv)
{
  printf("before rng..\n");
  for (int i = 0; i < 10; i++)
    printf("rng: %d\n", random());
  
  exit(0);
}
