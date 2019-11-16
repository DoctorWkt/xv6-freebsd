#include <stdio.h>
#include <sys/types.h>
//#include <xv6/user.h>

int
main(int argc, char *argv[])
{
  for (int i = 0; i < 10; i++)
    fprintf(stdout, "rng: %d\n", random(1, 10));
  exit();
}
