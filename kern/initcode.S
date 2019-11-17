# Initial process execs /etc/init.

#include <xv6/syscall.h>
#include <xv6/traps.h>

# exec(init, argv)
.globl start
start:
  pushl $argv
  pushl $init
  pushl $0  // where caller pc would be
  movl $SYS_exec, %eax
  int $T_SYSCALL

# for(;;) exit();
exit:
  movl $SYS_exit, %eax
  int $T_SYSCALL
  jmp exit

# char init[] = "/etc/init\0";
init:
  .string "/etc/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0

