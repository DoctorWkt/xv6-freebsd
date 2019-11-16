// halt the system.
#include <xv6/types.h>
#include <xv6/user.h>

int
main(void) {
  halt(0);
  exit();
}
