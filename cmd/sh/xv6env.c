// Because xv6 doesn't set the environ, we simulate it here
#include <stdio.h>
#include <string.h>
#define ENVLEN 100
#define ENV_FILE "/etc/env"
//#define ENV_FILE "/home/wkt/xv6-freebsd/fs/etc/env"
#define ENV_LINE_LEN 200

static char * xv6env[ENVLEN];
extern char **environ;

void loadxv6env()
{
  FILE *in;
  char env_line[ENV_LINE_LEN];
  int posn=0;

    // Try to open the env file
  if ((in=fopen(ENV_FILE, "r"))==NULL) return;

  while (fgets(env_line, ENV_LINE_LEN-1, in)!=NULL) {
    // Ignore comments
    if (env_line[0] == '#') continue;

    // Stop if we run out of room
    if (posn==ENVLEN) break;

    // Remove the newline
    env_line [ strlen(env_line) - 1] = '\0';

    // Copy the variable into the array
    xv6env[posn++]=strdup(env_line);
  }
  fclose(in);
  environ= xv6env;
  return;
}
