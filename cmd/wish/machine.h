/*      Minix 1.5 specific includes and defines
 *
 * $Revision: 1.1 $ $Date: 2016/07/22 12:03:23 $
 *
 */
#define MINIX1_7	/* Minix 1.7 */

#define USES_TERMIOS
#define PROTO
#define STDARG

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>	/* Only for perror */
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <termcap.h>
#include <dirent.h>
#include <time.h>
