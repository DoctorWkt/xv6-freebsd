/* A simple test for opendir()		Author: Warren Toomey */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  DIR *dirp;
  struct dirent *dent;

  if (argc != 2) {
    printf("Usage: odir-test <dirname>\n");
    exit(1);
  }

  dirp= opendir(argv[1]);
  if (dirp==NULL) {
    printf("opendir failed\n");
    exit(1);
  }

  while (1) {
    dent= readdir(dirp);
    if (dent==NULL) {
      printf("no more readdir\n");
      break;
    }
    // Skip empty directory entries
    if (dent->d_name[0]==0) continue;
    printf("%d: %s\n", dent->d_ino, &(dent->d_name));
  }

  if (closedir(dirp)!=0) {
    printf("closedir failed\n");
    exit(1);
  }
  exit(0);
}
