#include <unistd.h>

void sync(void) { return; }

unsigned int alarm(unsigned int seconds) { return(0); }

void logwtmp(void) { return; }

void updwtmp(void) { return; }

char *getlogin(void) { return("root"); }

void openlog(void) { return; }

void syslog(void) { return; }

void closelog(void) { return; }

void vsyslog(void) { return; }
