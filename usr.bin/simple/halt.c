// halt the system.
#include <xv6/types.h>
#include <xv6/user.h>

int
main(int argc, char** argv) {
  int status = 0;
  
  if (argc == 2) {
    status = atoi(argv[1]);
  }
  
  halt(status);
  exit();
}
