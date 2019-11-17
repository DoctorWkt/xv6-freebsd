#pragma once

#include <xv6/vfs.h>

struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe;
  struct inode *ip;
  uint off;
};

struct superblock sb[NDEV];

// table mapping major device number to
// device functions
struct devsw {
  int (*read)(struct inode*, char*, int);
  int (*write)(struct inode*, char*, int);
  int (*ioctl)(struct inode*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1	// /dev/console, /dev/serial
#define DISK    2	// /dev/disk0,   /dev/disk1
#define DEVNULL 3	// /dev/null,    /dev/zero

// lseek defines
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

//PAGEBREAK!
// Blank page.
