#pragma once

#include <xv6/param.h>

struct buf {
  int flags;
  uint dev;
  uint blockno;
  uint bsize;       // Block Size of this buffer
  struct buf *prev; // LRU cache list
  struct buf *next;
  struct buf *qnext; // disk queue
  uchar data[MAXBSIZE];
};
#define B_BUSY  0x1  // buffer is locked by some process
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

