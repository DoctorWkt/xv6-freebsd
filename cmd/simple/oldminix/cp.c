/* cp - copy files	Author: Andy Tanenbaum */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define TRANSFER_UNIT    16384
char cpbuf[TRANSFER_UNIT];

void cp_to_dir(int argc, char *argv[]);
void copyfile(FILE *in, FILE *out);
void usage();
int equal(char *s1, char *s2);
int match(char *s1, char *s2, int n);

int main(int argc, char *argv[])
{
  FILE *in, *out;
  struct stat sbuf;
  int m, s;

  if (argc < 3) usage();

  /* Get the status of the last named file.  See if it is a directory. */
  s = stat(argv[argc-1], &sbuf);
  m = sbuf.st_mode & S_IFMT;
  if (s >= 0 && m == S_IFDIR) {
	/* Last argument is a directory. */
	cp_to_dir(argc, argv);
  } else if (argc > 3) {
	/* More than 2 arguments and last one is not a directory. */
	usage();
  } else if (s < 0 || m==S_IFREG || m==S_IFCHR || m==S_IFBLK){
	/* Exactly two arguments.  Check for cp f1 f1. */
	if (equal(argv[1], argv[2])) {
		fprintf(stderr,"cp: cannot copy a file to itself\n");
		exit(-1);
	}

	/* Command is of the form cp f1 f2. */
	in = fopen(argv[1], "r");
  	if (in == NULL)
		{fprintf(stderr, "cannot open %s\n", argv[1]); exit(1);}
  	out = fopen(argv[2], "w");
  	if (out == NULL)
		{fprintf(stderr, "cannot create %s\n", argv[2]); exit(2);}
  	copyfile(in, out);
  } else {
	fprintf(stderr, "cannot copy to %s\n", argv[2]);
	exit(3);
  }
  exit(0);
}





void cp_to_dir(int argc, char *argv[])
{
  int i;
  FILE *in, *out;
  char dirname[256], *ptr, *dp;

  for (i = 1; i < argc - 1; i++) {
	in = fopen(argv[i], "r");
	if (in == NULL) {
		fprintf(stderr, "cannot open %s\n", argv[i]);
		continue;
	}

	ptr = argv[argc-1];
	dp = dirname;
	while (*ptr != 0) *dp++ = *ptr++;

	*dp++ = '/';
	ptr = argv[i];

	/* Concatenate dir and file name in dirname buffer. */
	while (*ptr != 0) ptr++;	/* go to end of file name */
	while (ptr > argv[i] && *ptr != '/') ptr--;	/* get last component*/
	if (*ptr == '/') ptr++;
	while (*ptr != 0) *dp++ = *ptr++;
	*dp++ = 0;
 	out = fopen(dirname, "w");
	if (out == NULL) {
		fprintf(stderr, "cannot create %s\n", dirname);
		continue;
	}
	copyfile(in, out);
  }
}


void copyfile(FILE *in, FILE *out)
{
  int n, m;

  do {
        n = fread(cpbuf, 1, TRANSFER_UNIT, in);
        if (n == 0 ) break;
        if (n > 0) {
                m = fwrite(cpbuf, 1, n, out);
                if (m != n) {
                        fprintf(stderr, "m != n, %d %d\n", m, n);
                        perror("cp");
                        exit(1);
                }
        }
  } while (n == TRANSFER_UNIT);
  fclose(in);
  fclose(out);
}

void usage()
{
  fprintf(stderr, "Usage:  cp f1 f2;  or  cp f1 ... fn d2\n");
  exit(-1);
}

int equal(char *s1, char *s2)
{
  while (1) {
	if (*s1 == 0 && *s2 == 0) return(1);
	if (*s1 != *s2) return(0);
	if (*s1 == 0 || *s2 == 0) return(0);
	s1++;
	s2++;
  }
}

int match(char *s1, char *s2, int n)
{
  while (n--) {
	if (*s1++ != *s2++) return(0);
  }
  return(1);
}

