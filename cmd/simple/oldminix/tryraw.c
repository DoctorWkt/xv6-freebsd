#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int main()
{
  char buf[1000];
  struct termios T;

#if 1
  tcgetattr(0, &T);
  cfmakeraw(&T);		// Turn off ICANON
  T.c_lflag |= ECHO;		// But leave ECHO on
  tcsetattr(0, TCSANOW, &T);
#else
  system("stty raw");
#endif

  while (1) {			// We should get 1 char
    int i=read(0, buf, 1000);	// at a time
    if (i<1) {
      write(2, ".", 1); sleep(1); 
    } else {
      if (buf[0]==4) exit(0);	// Stop on leading ctrl-D
      write(2, buf, i);
    }
  }
}
