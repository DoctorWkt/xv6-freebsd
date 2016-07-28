#include <sys/types.h>
#include <unistd.h>

pid_t getpgid(void) { return(getpid()); }
pid_t getpgrp(void) { return(getpid()); }
int setpgrp(pid_t a, pid_t b) { return(0); }
int setpgid(pid_t a, pid_t b) { return(0); }
pid_t getsid(pid_t a) { return(getpid()); }
pid_t tcgetpgrp(int fd) { return(getpid()); }
int tcsetpgrp(int fd, pid_t pgrp) { return(0); }
