#include <stdio.h>

int main(void)
{
  printf("\e[H\e[2J");
  fflush(stdout);
  
  return 0;
}
