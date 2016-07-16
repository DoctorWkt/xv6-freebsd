
/* A simple test for open()		Author: Warren Toomey */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
  int fd;

  if (argc != 2) {
    printf("Usage: open-test <dirname>\n");
    exit(1);
  }

  fd= open(argv[1], O_WRONLY|O_CREAT, 0777);
  if (fd==-1) {
    printf("open failed\n");
    exit(1);
  }
  write(fd, "Hello, world\n", 13);
  close(fd);

  fd= creat(argv[1], 0777);
  if (fd==-1) {
    printf("creat failed\n");
    exit(1);
  }
  write(fd, "Hello, world\n", 13);
  close(fd);

  exit(0);
}
