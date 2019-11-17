/* A simple man program.		Author: Warren Toomey */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef TEST
#define MANPATH "/home/wkt/xv6-freebsd/fs/usr/share/man/man"
#define CATPATH "/home/wkt/xv6-freebsd/fs/usr/share/man/man"
#define NROFF   "/usr/bin/nroff"
#define PAGER   "/usr/bin/less"
//#define PAGER   "/usr/bin/wc"
#else
#define MANPATH "/usr/share/man/man"
#define CATPATH "/usr/share/man/cat"
#define NROFF   "/bin/nroff"
#define PAGER   "/bin/less"
#endif

void tryman(int section, char *title)
{
  char path[200];
  int cpid, pipefd[2];

  snprintf(path, 200, "%s%d/%s.%d", MANPATH, section, title, section);
// printf("About to access %s\n", path); fflush(stdout);
  if (access(path, O_RDONLY)<0) return;
// printf("About to do %s -man %s | %s\n", NROFF, path, PAGER); fflush(stdout);

  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }

  cpid = fork();
  if (cpid == -1) {
    perror("fork");
    exit(1);
  }

  if (cpid == 0) {		// Child reads
    close(pipefd[1]);
    close(0);
    dup2(pipefd[0], 0);
    close(pipefd[0]);
    execl(PAGER, PAGER, NULL);
  } else {			// Parent writes
    close(pipefd[0]);
    close(1);
    dup2(pipefd[1], 1);
    close(pipefd[1]);
    execl(NROFF, NROFF, "-man", path, NULL);
  }
  exit(-1);			// execl failed
}

int main(int argc, char *argv[])
{
  int section=0;
  char *title;
  int i;

  if ((argc<2) || (argc>3)) {
    fprintf(stderr, "Usage: man [section] title\n"); exit(1);
  }
  title=argv[1];
  if (argc==3) {
    section= atoi(argv[1]);
    title= argv[2];
  }

  // We got a specific section, try it
  if (section) tryman(section, title);
  
  // No luck, try each man section
  for (i=1; i <=8; i++)
    tryman(i, title);

  fprintf(stderr, "No man page for %s\n", title);
  exit(1);
}
