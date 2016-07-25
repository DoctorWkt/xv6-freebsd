#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  char *value;

  if (argc!=2) { fprintf(stderr, "Usage: getenv varname\n"); exit(1); }

  value= getenv(argv[1]);
  if (value==NULL) {
    puts("No value");
  } else {
    puts(value);
  }
  exit(0);
}
