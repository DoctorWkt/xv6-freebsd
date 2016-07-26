#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

/* This comes from https://github.com/nyuichi/xv6 */

int tcgetattr(int fd, struct termios *termios_p)
{
  return ioctl(fd, TCGETA, (void *)termios_p);
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
  return ioctl(fd, TCSETA, (void *)termios_p);
}

void cfmakeraw(struct termios *termios_p)
{
  // Ignore optional_actions
  termios_p->c_lflag = 0;
}

/* This comes from Minix 2.0.2 */
int isatty(int fd)
{
  struct termios dummy;

  return(tcgetattr(fd, &dummy) == 0);
}

int tcsendbreak(int fd, int len)
{ return(0); }
