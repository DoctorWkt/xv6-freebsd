#include <sys/types.h>
#include <sys/stat.h>

/* xv6 doesn't have this */
mode_t umask(Mode_t cmask)
{
 return(0777);
}
