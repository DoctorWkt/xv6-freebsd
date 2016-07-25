#include <sys/types.h>
#include <sys/stat.h>

/* xv6 doesn't have this */
mode_t umask(mode_t cmask)
{
 return(0777);
}
