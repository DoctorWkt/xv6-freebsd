#include <xv6/types.h>
#include <xv6/defs.h>
#include <xv6/param.h>
#include <xv6/spinlock.h>
#include <xv6/vfs.h>
#include <xv6/buf.h>
#include <xv6/s5.h>

// Simple logging that allows concurrent FS system calls.
//
// A log transaction contains the updates of multiple FS system
// calls. The logging system only commits when there are
// no FS system calls active. Thus there is never
// any reasoning required about whether a commit might
// write an uncommitted system call's updates to disk.
//
// A system call should call begin_op()/end_op() to mark
// its start and end. Usually begin_op() just increments
// the count of in-progress FS system calls and returns.
// But if it thinks the log is close to running out, it
// sleeps until the last outstanding end_op() commits.
//
// The log is a physical re-do log containing disk blocks.
// The on-disk log format:
//   header block, containing block #s for block A, B, C, ...
//   block A
//   block B
//   block C
//   ...
// Log appends are synchronous.

// Contents of the header block, used for both the on-disk header block
// and to keep track in memory of logged block# before commit.
struct logheader {
  int n;
  int block[LOGSIZE];
};

#define NLOG 3   // Max number of active logs
#define LOGENABLED 1

struct log {
  struct spinlock lock;
  int start;
  int size;
  int outstanding; // how many FS sys calls are executing.
  int committing;  // in commit(), please wait.
  int dev;
  int flag;
  struct logheader lh;
};
struct log log[NLOG];

static void recover_from_log(void);
static void commit();

void
initlog(int dev)
{
  if (sizeof(struct logheader) >= BSIZE)
    panic("initlog: too big logheader");

  char devnum[3];
  itoa(dev, devnum);
  char logname[16];

  strconcat(logname, "log ", devnum);

  struct superblock sb;
  initlock(&log[dev].lock, logname);

  s5_readsb(dev, &sb);
  struct s5_superblock *s5sb = sb.fs_info;

  log[dev].start = s5sb->logstart;
  log[dev].size = s5sb->nlog;
  log[dev].dev = dev;
  log[dev].flag |= LOGENABLED;
  recover_from_log();
}

// Copy committed blocks from log to their home location
static void
install_trans(int dev)
{
  int tail;
  if (!log[dev].flag & LOGENABLED) return;

  for (tail = 0; tail < log[dev].lh.n; tail++) {
    struct buf *lbuf = bread(log[dev].dev, log[dev].start+tail+1); // read log block
    struct buf *dbuf = bread(log[dev].dev, log[dev].lh.block[tail]); // read dst
    memmove(dbuf->data, lbuf->data, BSIZE);  // copy block to dst
    bwrite(dbuf);  // write dst to disk
    brelse(lbuf);
    brelse(dbuf);
  }
}

// Read the log header from disk into the in-memory log header
static void
read_head(int dev)
{
  if (!log[dev].flag & LOGENABLED) return;

  struct buf *buf = bread(log[dev].dev, log[dev].start);
  struct logheader *lh = (struct logheader *) (buf->data);
  int i;
  log[dev].lh.n = lh->n;
  for (i = 0; i < log[dev].lh.n; i++) {
    log[dev].lh.block[i] = lh->block[i];
  }
  brelse(buf);
}

// Write in-memory log header to disk.
// This is the true point at which the
// current transaction commits.
static void
write_head(int dev)
{
  if (!log[dev].flag & LOGENABLED) return;

  struct buf *buf = bread(log[dev].dev, log[dev].start);
  struct logheader *hb = (struct logheader *) (buf->data);
  int i;
  hb->n = log[dev].lh.n;
  for (i = 0; i < log[dev].lh.n; i++) {
    hb->block[i] = log[dev].lh.block[i];
  }
  bwrite(buf);
  brelse(buf);
}

static void
recover_from_log(void)
{
  int i;

  for (i = 0; i < NLOG; i++) {
    if (!log[i].flag & LOGENABLED) continue;

    read_head(i);
    install_trans(i); // if committed, copy from log to disk
    log[i].lh.n = 0;
    write_head(i); // clear the log
  }
}

// called at the start of each FS system call.
void
begin_op(void)
{
  int i;

  for (i = 0; i < NLOG; i++) {
    if (!log[i].flag & LOGENABLED) continue;

    acquire(&log[i].lock);
    while(1){
      if(log[i].committing){
        sleep(&log[i], &log[i].lock);
      } else if(log[i].lh.n + (log[i].outstanding+1)*MAXOPBLOCKS > LOGSIZE){
        // this op might exhaust log space; wait for commit.
        sleep(&log[i], &log[i].lock);
      } else {
        log[i].outstanding += 1;
        release(&log[i].lock);
        break;
      }
    }
  }
}

// called at the end of each FS system call.
// commits if this was the last outstanding operation.
void
end_op(void)
{
  int i;

  for (i = 0; i < NLOG; i++) {
    if (!log[i].flag & LOGENABLED) continue;

    int do_commit = 0;

    acquire(&log[i].lock);
    log[i].outstanding -= 1;
    if(log[i].committing)
      panic("log.committing");
    if(log[i].outstanding == 0){
      do_commit = 1;
      log[i].committing = 1;
    } else {
      // begin_op() may be waiting for log space.
      wakeup(&log[i]);
    }
    release(&log[i].lock);

    if(do_commit){
      // call commit w/o holding locks, since not allowed
      // to sleep with locks.
      commit(i);
      acquire(&log[i].lock);
      log[i].committing = 0;
      wakeup(&log[i]);
      release(&log[i].lock);
    }
  }
}

// Copy modified blocks from cache to log.
static void
write_log(int dev)
{
  int tail;

  if (!log[dev].flag & LOGENABLED) return;

  for (tail = 0; tail < log[dev].lh.n; tail++) {
    struct buf *to = bread(log[dev].dev, log[dev].start+tail+1); // log block
    struct buf *from = bread(log[dev].dev, log[dev].lh.block[tail]); // cache block
    memmove(to->data, from->data, BSIZE);
    bwrite(to);  // write the log
    brelse(from);
    brelse(to);
  }
}

static void
commit(int dev)
{
  if (log[dev].lh.n > 0) {
    write_log(dev);     // Write modified blocks from cache to log
    write_head(dev);    // Write header to disk -- the real commit
    install_trans(dev); // Now install writes to home locations
    log[dev].lh.n = 0;
    write_head(dev);    // Erase the transaction from the log
  }
}

// Caller has modified b->data and is done with the buffer.
// Record the block number and pin in the cache with B_DIRTY.
// commit()/write_log() will do the disk write.
//
// log_write() replaces bwrite(); a typical use is:
//   bp = bread(...)
//   modify bp->data[]
//   log_write(bp)
//   brelse(bp)
void
log_write(struct buf *b)
{
  int i;

  if (!log[b->dev].flag & LOGENABLED) return;

  if (log[b->dev].lh.n >= LOGSIZE || log[b->dev].lh.n >= log[b->dev].size - 1)
    panic("too big a transaction");
  if (log[b->dev].outstanding < 1)
    panic("log_write outside of trans");

  acquire(&log[b->dev].lock);
  for (i = 0; i < log[b->dev].lh.n; i++) {
    if (log[b->dev].lh.block[i] == b->blockno)   // log absorbtion
      break;
  }
  log[b->dev].lh.block[i] = b->blockno;
  if (i == log[b->dev].lh.n)
    log[b->dev].lh.n++;
  b->flags |= B_DIRTY; // prevent eviction
  release(&log[b->dev].lock);
}

