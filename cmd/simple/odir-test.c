/* A simple test for opendir()		Author: Warren Toomey */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  DIR *dirp;
  struct dirent *dent;
  struct stat s;

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

    // Try to stat the file
    if (stat(dent->d_name, &s)==0) {
	// mode links uid gid size mtime name
      printf("0%06o %d %d %d %6d %06d %s\n",
	s.st_mode, s.st_nlink, s.st_uid, s.st_gid,
	s.st_size, s.st_mtime, &(dent->d_name));
    } else {
      printf("%d: %s\n", dent->d_ino, &(dent->d_name));
    }
  }

  if (closedir(dirp)!=0) {
    printf("closedir failed\n");
    exit(1);
  }
  exit(0);
}
