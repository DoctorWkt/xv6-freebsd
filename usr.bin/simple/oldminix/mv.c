/* mv: Move files		Author: Warren Toomey */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Move something to a new location
void do_move(char *from, char *to)
{
  // Unlink whatever is there
  unlink(to);

  // Link old to new
  if (link(from, to)!=0) {
    fprintf(stderr, "Unable to link %s to %s\n", from, to);
    exit(1);
  }

  // Now try to unlink the old one
  if (unlink(from)!=0) {
    fprintf(stderr, "Unable to unlink %s\n", from);
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  struct stat sb;
  char fulldest[200];

  if (argc<3) {
    fprintf(stderr, "Usage: mv file1 file, or mv file file ... dir\n");
    exit(1);
  }

  // See if the last argument is a directory
  if ((stat(argv[argc-1], &sb)==0) && ((sb.st_mode & S_IFMT) == S_IFDIR)) {

    // Yes, link each argument into that directory
    for (int i= 1; i < argc-1; i++) {

      // Append the from file's name to the directory name
      strcpy(fulldest, argv[argc-1]);
      strcat(fulldest, "/");
      strcat(fulldest, argv[i]);
      do_move(argv[i], fulldest);
    }
  } else {
    // We should only have two files
    if (argc != 3) {
      fprintf(stderr, "Usage: mv file1 file, or mv file file ... dir\n");
      exit(1);
    }
    do_move(argv[1], argv[2]);
  }
  exit(0);
}
