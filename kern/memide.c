// Fake IDE disk; stores blocks in memory.
// Useful for running kernel without scratch disk.

#include <xv6/types.h>
#include <xv6/defs.h>
#include <xv6/param.h>
#include <xv6/mmu.h>
#include <xv6/proc.h>
#include <xv6/x86.h>
#include <xv6/traps.h>
#include <xv6/spinlock.h>
#include <xv6/fs.h>
#include <xv6/buf.h>

extern uchar _binary_fs_img_start[], _binary_fs_img_size[];

static int disksize;
static uchar *memdisk;

void
ideinit(void)
{
  memdisk = _binary_fs_img_start;
  disksize = (uint)_binary_fs_img_size/BSIZE;
}

// Interrupt handler.
void
ideintr(void)
{
  // no-op
}

// Sync buf with disk.
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
void
iderw(struct buf *b)
{
  uchar *p;

  if(!(b->flags & B_BUSY))
    panic("iderw: buf not busy");
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
    panic("iderw: nothing to do");
  if(b->dev != 1)
    panic("iderw: request not for disk 1");
  if(b->blockno >= disksize)
    panic("iderw: block out of range");

  p = memdisk + b->blockno*BSIZE;

  if(b->flags & B_DIRTY){
    b->flags &= ~B_DIRTY;
    memmove(p, b->data, BSIZE);
  } else
    memmove(b->data, p, BSIZE);
  b->flags |= B_VALID;
}
