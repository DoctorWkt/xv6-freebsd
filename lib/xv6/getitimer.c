#include <sys/time.h>

int getitimer(int which, struct itimerval *curr_value) { return(0); }
int setitimer(int which, const struct itimerval *new_value,
                     struct itimerval *old_value) { return(0); }
