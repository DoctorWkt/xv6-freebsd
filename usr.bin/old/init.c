// init: The initial user-level program

#include <xv6/types.h>
#include <xv6/stat.h>
#include <xv6/user.h>
#include <xv6/fcntl.h>

char *argv[] = { "sh", 0 };

void spawnshell(char *devname)
{
  int pid, wpid;

  open(devname, O_RDWR);
  dup(0);  // stdout
  dup(0);  // stderr

  for(;;){
    printf(1, "init: starting /bin/sh\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      exec("/bin/sh", argv);
      printf(1, "init: exec /bin/sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      ; 		// Clean up zombies
  }
}

int
main(void)
{
  if(open("/dev/console", O_RDWR) < 0){
    mknod("/dev/console", 1, 0);
  }
  if(open("/dev/serial", O_RDWR) < 0){
    mknod("/dev/serial", 1, 1);
  }
  /*
  if(open("/dev/disk0", O_RDWR) < 0){
    mknod("/dev/disk0", 2, 0);
  }
  if(open("/dev/disk1", O_RDWR) < 0){
    mknod("/dev/disk1", 2, 1);
    }*/
  if(open("/dev/null", O_RDWR) < 0){
    mknod("/dev/null", 3, 0);
  }
  if(open("/dev/zero", O_RDWR) < 0){
    mknod("/dev/zero", 4, 0);
  }
  if(open("/dev/random", O_RDWR) < 0){
    mknod("/dev/random", 5, 0);
  }

  switch(fork()){
    case 0:  spawnshell("/dev/console");
    default: spawnshell("/dev/serial");
  }
}
