/* mktemp - make a name for a temporary file */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

char *mktemp(char *template)
{
  int k;
  char *p;
  time_t tim;
  int fd;

  tim = time(NULL);		/* get time as semi-unique number */
  p = template;
  while (*p != 0) p++;		/* find end of string */

  /* Replace XXXXXX at end of template with a letter, then as many of the
   * trailing digits of the tim as fit.
   */
  while (*--p == 'X') {
	*p = '0' + (tim % 10);
	tim /= 10;
  }
  if (*++p != 0) {
	for (k = 'a'; k <= 'z'; k++) {
		*p = k;
		fd= open(template, O_RDONLY);
		if (fd < 0) return(template);
		close(fd);
	}
  }
  return("/");
}

int mkstemp(char *template)
{
  char *name= mktemp(template);
  if (name==NULL) return(-1);
  return(open(name, O_RDWR));
}
