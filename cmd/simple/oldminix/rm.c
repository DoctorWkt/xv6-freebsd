/* rm - remove files			Author: Adri Koppes */

#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/wait.h>
#include <dirent.h>

int errors = 0;
int fflag = 0;
int iflag = 0;
int rflag = 0;

void usage();
void do_remove(char *name);
void stderr3(char *s1, char *s2, char *s3);
void octal(unsigned int num);
void rem_dir(char *name);
int confirm(void);

int exec(const char *path, char *const argv[]);

int execl(const char *path, const char *arg1, ...)
{
        /* Assumption:  The C-implementation for this machine pushes
         * function arguments downwards on the stack making a perfect
         * argument array.  Luckily this is almost always so.
         */
        return exec(path, (char * const *) &arg1);
}

int main(int argc, char *argv[])
{
  int exstatus=0;
  char *opt;

  if (argc < 2) usage();
  argv++;
  argc--;
  while (**argv == '-') {
	opt = *argv;
	while (*++opt != '\0') switch (*opt) {
		    case 'f':	fflag++;	break;
		    case 'i':	iflag++;	break;
		    case 'r':	rflag++;	break;
		    default:
			std_err("rm: unknown option\n");
			usage();
			break;
		}
	argc--;
	argv++;
  }
  if (argc < 1) usage();
  while (argc--) { do_remove(argv[0]); argv++; }
  exstatus = (errors == 0 ? 0 : 1);
  if (fflag) exstatus = 0;
  exit(exstatus);
}

void usage()
{
  std_err("Usage: rm [-fir] file\n");
  exit(1);
}

void do_remove(char *name)
{
  struct stat s;
  char rname[128];
  DIR *D;
  struct dirent *dent;

  if (stat(name, &s)) {
	if (!fflag) stderr3("rm: ", name, " non-existent\n");
	errors++;
	return;
  }
  if (iflag) {
	stderr3("rm: remove ", name, "? ");
	if (!confirm()) return;
  }
  if ((s.st_mode & S_IFMT) == S_IFDIR) {
	if (rflag) {
		D= opendir(name);
    		if (D==NULL) {
      		  	printf("%s: unable to opendir\n", name);
			errors++;
      			return;
		}
		while ((dent=readdir(D))!=NULL) {
			if ((dent->d_name[0]!='\0')
			    && strcmp("..", dent->d_name)
			    && strcmp(".", dent->d_name)) {
				strcpy(rname, name);
				strcat(rname, "/");
				strncat(rname, dent->d_name, DIRSIZ);
				do_remove(rname);
			}
		}
		closedir(D);
		rem_dir(name);
	} else {
		if (!fflag) stderr3("rm: ", name, " is a directory\n");
		errors++;
		return;
	}
  } else {
	if (access(name, 2) && !fflag) {
		stderr3("rm: remove ", name, " (mode = ");
		octal(s.st_mode & 0777);
		std_err(") ? ");
		if (!confirm()) return;
	}
	if (unlink(name)) {
		if (!fflag) stderr3("rm: ", name, " not removed\n");
		errors++;
	}
  }
}

void rem_dir(char *name)
{
  int status;

  switch (fork()) {
      case -1:
	std_err("rm: can't fork\n");
	errors++;
	return;
      case 0:
	execl("/bin/rmdir", "rmdir", name, (char *) 0);
	execl("/usr/bin/rmdir", "rmdir", name, (char *) 0);
	std_err("rm: can't exec rmdir\n");
	exit(1);
      default:
	wait(&status);
	errors += status;
  }
}

int confirm(void)
{
  char c, t;
  read(0, &c, 1);
  t = c;
  do
	read(0, &t, 1);
  while (t != '\n' && t != -1);
  return(c == 'y' || c == 'Y');
}

void octal(unsigned int num)
{
  char a[4];

  a[0] = (((num >> 6) & 7) + '0');
  a[1] = (((num >> 3) & 7) + '0');
  a[2] = ((num & 7) + '0');
  a[3] = 0;
  std_err(a);
}

void stderr3(char *s1, char *s2, char *s3)
{
  std_err(s1);
  std_err(s2);
  std_err(s3);
}
