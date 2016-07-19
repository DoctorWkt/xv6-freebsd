/* tee - pipe fitting			Author: Paul Polderman */

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define	MAXFD	18
#define CHUNK_SIZE	4096

int fd[MAXFD];

_PROTOTYPE(int main, (int argc, char **argv));

int main(argc, argv)
int argc;
char **argv;
{
  char aflag = 0;
  char buf[CHUNK_SIZE];
  int i, s, n;

  argv++;
  --argc;
  while (argc > 0 && argv[0][0] == '-') {
	switch (argv[0][1]) {
	    case 'a':		/* Append to outputfile(s), instead of
			 * overwriting them. */
		aflag++;
		break;
	    default:
		fprintf(stderr,"Usage: tee [-a] [files].\n");
		exit(1);
	}
	argv++;
	--argc;
  }
  fd[0] = 1;			/* Always output to stdout. */
  for (s = 1; s < MAXFD && argc > 0; --argc, argv++, s++) {
	if (aflag && (fd[s] = open(*argv, O_RDWR)) >= 0) {
		lseek(fd[s], 0L, SEEK_END);
		continue;
	} else {
		if ((fd[s] = creat(*argv, 0666)) >= 0) continue;
	}
	fprintf(stderr,"Cannot open output file: %s\n", *argv);
	exit(2);
  }

  while ((n = read(0, buf, CHUNK_SIZE)) > 0) {
	for (i = 0; i < s; i++) write(fd[i], buf, n);
  }

  for (i = 0; i < s; i++)	/* Close all fd's */
	close(fd[i]);
  return(0);
}
