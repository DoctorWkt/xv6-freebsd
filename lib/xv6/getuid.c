#include <unistd.h>
#include <sys/types.h>

/* Every user is root in xv6 */
uid_t getuid(void) { return(0); }
uid_t geteuid(void) { return(0); }
gid_t getgid(void) { return(0); }
gid_t getegid(void) { return(0); }
int setuid(uid_t uid) { return(0); }
int setgid(gid_t gid) { return(0); }
