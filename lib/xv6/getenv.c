/* getenv(): get environment variable's value	Author: Warren Toomey */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENV_FILE "/etc/env"

#define LINE_LEN 200
char _env_line[LINE_LEN];

char *getenv(const char *key)
{
  FILE *in;
  int found=0;
  char *valptr=NULL;
  char *nlptr;

  // Try to open the env file
  if ((in=fopen(ENV_FILE, "r"))==NULL) return(NULL);
  // printf("In getenv, looking for %s\n", key);
  // fflush(stdout);

  // Read lines from the file
  while (fgets(_env_line, LINE_LEN-1, in)!=NULL) {
    // Skip those starting with '#'
    if (_env_line[0]=='#') continue;
    // printf("line is %s\n", _env_line);

    // Find an '=' character
    if ((valptr=strchr(_env_line, '='))!=NULL) {
      // Break the line into two strings
      *(valptr++)= '\0';
    // printf("We now have %s and %s\n",  _env_line,  valptr);

      // We have a match on the key
      if (!strcmp(key, _env_line)) {
	found=1;
	// printf("A match against %s\n", key);
	// Remove the newline
	nlptr=strrchr(valptr, '\n');
	if (nlptr!=NULL) *nlptr='\0';
	break;
      }
    }
  }

  fclose(in);
  if (!found) return(NULL);
  return(strdup(valptr));
}

int setenv(name, value, rewrite)
        register const char *name;
        register const char *value;
        int rewrite;
{
  return(-1);
}
