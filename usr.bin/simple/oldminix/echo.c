/* echo, used by xargs.		Author: Warren Toomey */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if (argc==1) { putchar('\n'); exit(0); }
  for (int i=1; i < argc-1; i++)
    printf("%s ", argv[i]);
  printf("%s\n", argv[argc-1]);
  exit(0);
}
