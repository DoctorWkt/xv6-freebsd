// The s5 filesystem implementation

#include <xv6/types.h>
#include <xv6/defs.h>
#include <xv6/param.h>
#include <xv6/stat.h>
#include <xv6/mmu.h>
#include <xv6/proc.h>
#include <xv6/spinlock.h>
#include <xv6/vfs.h>
#include <xv6/buf.h>
#include <xv6/file.h>
#include <xv6/vfsmount.h>
#include <xv6/s5.h>

/*
 * Its is a pool to allocate s5 inodes structs.
 * We use it becase we don't have a kmalloc function.
 * With an kmalloc implementatios, it need to be removed.
 */
static struct {
  struct spinlock lock;
  struct s5_inode s5_i_entry[NINODE];
} s5_inode_pool;

struct s5_inode*
alloc_s5_inode()
{
  struct s5_inode *ip;

  acquire(&s5_inode_pool.lock);
  for (ip = &s5_inode_pool.s5_i_entry[0]; ip < &s5_inode_pool.s5_i_entry[NINODE]; ip++) {
    if (ip->flag == S5_INODE_FREE) {
      ip->flag |= S5_INODE_USED;
      release(&s5_inode_pool.lock);

      return ip;
    }
  }
  release(&s5_inode_pool.lock);

  return 0;
}

static struct {
  struct spinlock lock;
  struct s5_superblock sb[MAXVFSSIZE];
} s5_sb_pool; // It is a Pool of S5 Superblock Filesystems

struct s5_superblock*
alloc_s5_sb()
{
  struct s5_superblock *sb;

  acquire(&s5_sb_pool.lock);
  for (sb = &s5_sb_pool.sb[0]; sb < &s5_sb_pool.sb[MAXVFSSIZE]; sb++) {
    if (sb->flags == S5_SB_FREE) {
      sb->flags |= S5_SB_USED;
      release(&s5_sb_pool.lock);

      return sb;
    }
  }
  release(&s5_sb_pool.lock);

  return 0;
}

struct vfs_operations s5_ops = {
  .fs_init = &s5fs_init,
  .mount   = &s5_mount,
  .unmount = &s5_unmount,
  .getroot = &s5_getroot,
  .readsb  = &s5_readsb,
  .ialloc  = &s5_ialloc,
  .balloc  = &s5_balloc,
  .bzero   = &s5_bzero,
  .bfree   = &s5_bfree,
  .brelse  = &brelse,
  .bwrite  = &bwrite,
  .bread   = &bread,
  .namecmp = &s5_namecmp
};

struct inode_operations s5_iops = {
  .dirlookup  = &s5_dirlookup,
  .iupdate    = &s5_iupdate,
  .itrunc     = &s5_itrunc,
  .cleanup    = &s5_cleanup,
  .bmap       = &s5_bmap,
  .ilock      = &s5_ilock,
  .iunlock    = &generic_iunlock,
  .stati      = &generic_stati,
  .readi      = &s5_readi,
  .writei     = &s5_writei,
  .dirlink    = &generic_dirlink,
  .unlink     = &s5_unlink,
  .isdirempty = &s5_isdirempty
};

struct filesystem_type s5fs = {
  .name = "s5",
  .ops = &s5_ops,
  .iops = &s5_iops
};

int
inits5fs(void)
{
  initlock(&s5_sb_pool.lock, "s5_sb_pool");
  initlock(&s5_inode_pool.lock, "s5_inode_pool");
  return register_fs(&s5fs);
}

int
s5fs_init(void)
{
  return 0;
}

int
s5_mount(struct inode *devi, struct inode *ip)
{
  struct mntentry *mp;

  // Read the Superblock
  s5_ops.readsb(devi->minor, &sb[devi->minor]);

  // Read the root device
  struct inode *devrtip = s5_ops.getroot(devi->major, devi->minor);

  acquire(&mtable.lock);
  for (mp = &mtable.mpoint[0]; mp < &mtable.mpoint[MOUNTSIZE]; mp++) {
    // This slot is available
    if (mp->flag == 0) {
found_slot:
      mp->dev = devi->minor;
      mp->m_inode = ip;
      mp->pdata = &sb[devi->minor];
      mp->flag |= M_USED;
      mp->m_rtinode = devrtip;

      release(&mtable.lock);

      initlog(devi->minor);
      return 0;
    } else {
      // The disk is already mounted
      if (mp->dev == devi->minor) {
        release(&mtable.lock);
        return -1;
      }

      if (ip->dev == mp->m_inode->dev && ip->inum == mp->m_inode->inum)
        goto found_slot;
    }
  }
  release(&mtable.lock);

  return -1;
}

int
s5_unmount(struct inode *devi)
{
  return 0;
}

struct inode *
s5_getroot(int major, int minor)
{
  return s5_iget(minor, ROOTINO);
}

void
s5_readsb(int dev, struct superblock *sb)
{
  struct buf *bp;
  struct s5_superblock *s5sb;

  if((sb->flags & SB_NOT_LOADED) == 0) {
    s5sb = alloc_s5_sb(); // Allocate a new S5 sb struct to the superblock.
  } else{
    s5sb = sb->fs_info;
  }

  // These sets are needed because of bread
  sb->major = IDEMAJOR;
  sb->minor = dev;
  sb->blocksize = BSIZE;

  bp = s5_ops.bread(dev, 1);
  memmove(s5sb, bp->data, sizeof(*s5sb) - sizeof(s5sb->flags));
  s5_ops.brelse(bp);

  sb->fs_info = s5sb;
}

struct inode*
s5_ialloc(uint dev, short type)
{
  int inum;
  struct buf *bp;
  struct dinode *dip;
  struct s5_superblock *s5sb;

  s5sb = sb[dev].fs_info;

  for(inum = 1; inum < s5sb->ninodes; inum++){
    bp = s5_ops.bread(dev, IBLOCK(inum, (*s5sb)));
    dip = (struct dinode*)bp->data + inum%IPB;
    if(dip->type == 0){  // a free inode
      memset(dip, 0, sizeof(*dip));
      dip->type = type;
      log_write(bp);   // mark it allocated on the disk
      s5_ops.brelse(bp);
      return s5_iget(dev, inum);
    }
    s5_ops.brelse(bp);
  }
  panic("ialloc: no inodes");
}

uint
s5_balloc(uint dev)
{
  int b, bi, m;
  struct buf *bp;
  struct s5_superblock *s5sb;

  s5sb = sb[dev].fs_info;
  bp = 0;
  for (b = 0; b < s5sb->size; b += BPB) {
    bp = s5_ops.bread(dev, BBLOCK(b, (*s5sb)));
    for (bi = 0; bi < BPB && b + bi < s5sb->size; bi++) {
      m = 1 << (bi % 8);
      if ((bp->data[bi/8] & m) == 0) {  // Is block free?
        bp->data[bi/8] |= m;  // Mark block in use.
        log_write(bp);
        s5_ops.brelse(bp);
        s5_ops.bzero(dev, b + bi);
        return b + bi;
      }
    }
    s5_ops.brelse(bp);
  }
  panic("balloc: out of blocks");
}

void
s5_bzero(int dev, int bno)
{
  struct buf *bp;

  bp = s5_ops.bread(dev, bno);
  memset(bp->data, 0, BSIZE);
  log_write(bp);
  s5_ops.brelse(bp);
}

void
s5_bfree(int dev, uint b)
{
  struct buf *bp;
  int bi, m;
  struct s5_superblock *s5sb;

  s5sb = sb[dev].fs_info;
  s5_ops.readsb(dev, &sb[dev]);
  bp = s5_ops.bread(dev, BBLOCK(b, (*s5sb)));
  bi = b % BPB;
  m = 1 << (bi % 8);
  if((bp->data[bi/8] & m) == 0)
    panic("freeing free block");
  bp->data[bi/8] &= ~m;
  log_write(bp);
  s5_ops.brelse(bp);
}

struct inode*
s5_dirlookup(struct inode *dp, char *name, uint *poff)
{
  uint off, inum;
  struct dirent de;

  if(dp->type == T_FILE || dp->type == T_DEV)
    panic("dirlookup not DIR");

  for(off = 0; off < dp->size; off += sizeof(de)){
    if(s5_iops.readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("dirlink read");
    if(de.inum == 0)
      continue;
    if(s5_ops.namecmp(name, de.name) == 0){
      // entry matches path element
      if(poff)
        *poff = off;
      inum = de.inum;
      return s5_iget(dp->dev, inum);
    }
  }

  return 0;
}

void
s5_iupdate(struct inode *ip)
{
  struct buf *bp;
  struct dinode *dip;
  struct s5_superblock *s5sb;
  struct s5_inode *s5ip;

  s5ip = ip->i_private;
  s5sb = sb[ip->dev].fs_info;
  bp = s5_ops.bread(ip->dev, IBLOCK(ip->inum, (*s5sb)));
  dip = (struct dinode*)bp->data + ip->inum%IPB;
  dip->type = ip->type;
  dip->major = ip->major;
  dip->minor = ip->minor;
  dip->nlink = ip->nlink;
  dip->size = ip->size;
  memmove(dip->addrs, s5ip->addrs, sizeof(s5ip->addrs));
  log_write(bp);
  s5_ops.brelse(bp);
}

void
s5_itrunc(struct inode *ip)
{
  int i, j;
  struct buf *bp;
  uint *a;
  struct s5_inode *s5ip;

  s5ip = ip->i_private;

  for(i = 0; i < NDIRECT; i++){
    if(s5ip->addrs[i]){
      s5_ops.bfree(ip->dev, s5ip->addrs[i]);
      s5ip->addrs[i] = 0;
    }
  }

  if(s5ip->addrs[NDIRECT]){
    bp = s5_ops.bread(ip->dev, s5ip->addrs[NDIRECT]);
    a = (uint*)bp->data;
    for (j = 0; j < NINDIRECT; j++) {
      if (a[j])
        s5_ops.bfree(ip->dev, a[j]);
    }
    s5_ops.brelse(bp);
    s5_ops.bfree(ip->dev, s5ip->addrs[NDIRECT]);
    s5ip->addrs[NDIRECT] = 0;
  }

  ip->size = 0;
  s5_iops.iupdate(ip);
}

void
s5_cleanup(struct inode *ip)
{
  memset(ip->i_private, 0, sizeof(struct s5_inode));
}

uint
s5_bmap(struct inode *ip, uint bn)
{
  uint addr, *a;
  struct buf *bp;
  struct s5_inode *s5ip;

  s5ip = ip->i_private;

  if(bn < NDIRECT){
    if((addr = s5ip->addrs[bn]) == 0)
      s5ip->addrs[bn] = addr = s5_ops.balloc(ip->dev);
    return addr;
  }
  bn -= NDIRECT;

  if(bn < NINDIRECT){
    // Load indirect block, allocating if necessary.
    if((addr = s5ip->addrs[NDIRECT]) == 0)
      s5ip->addrs[NDIRECT] = addr = s5_ops.balloc(ip->dev);
    bp = s5_ops.bread(ip->dev, addr);
    a = (uint*)bp->data;
    if((addr = a[bn]) == 0){
      a[bn] = addr = s5_ops.balloc(ip->dev);
      log_write(bp);
    }
    s5_ops.brelse(bp);
    return addr;
  }

  panic("bmap: out of range");
}

void
s5_ilock(struct inode *ip)
{
  struct buf *bp;
  struct dinode *dip;
  struct s5_superblock *s5sb;
  struct s5_inode *s5ip;

  s5ip = ip->i_private;

  s5sb = sb[ip->dev].fs_info;

  if(ip == 0 || ip->ref < 1)
    panic("s5 ilock");

  acquire(&icache.lock);
  while (ip->flags & I_BUSY)
    sleep(ip, &icache.lock);
  ip->flags |= I_BUSY;
  release(&icache.lock);

  if (!(ip->flags & I_VALID)) {
    bp = s5_ops.bread(ip->dev, IBLOCK(ip->inum, (*s5sb)));
    dip = (struct dinode*)bp->data + ip->inum%IPB;
    ip->type = dip->type;
    ip->major = dip->major;
    ip->minor = dip->minor;
    ip->nlink = dip->nlink;
    ip->size = dip->size;
    memmove(s5ip->addrs, dip->addrs, sizeof(s5ip->addrs));
    s5_ops.brelse(bp);
    ip->flags |= I_VALID;
    if (ip->type == 0)
      panic("s5 ilock: no type");
  }
}

int
s5_readi(struct inode *ip, char *dst, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if(ip->type == T_DEV){
    if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
      return -1;
    return devsw[ip->major].read(ip, dst, n);
  }

  if(off > ip->size || off + n < off)
    return -1;
  if(off + n > ip->size)
    n = ip->size - off;

  for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
    bp = ip->fs_t->ops->bread(ip->dev, ip->iops->bmap(ip, off/BSIZE));
    m = min(n - tot, BSIZE - off%BSIZE);
    memmove(dst, bp->data + off%BSIZE, m);
    ip->fs_t->ops->brelse(bp);
  }
  return n;
}

int
s5_writei(struct inode *ip, char *src, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if(ip->type == T_DEV){
    if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
      return -1;
    return devsw[ip->major].write(ip, src, n);
  }

  if(off > ip->size || off + n < off)
    return -1;
  if(off + n > MAXFILE*BSIZE)
    return -1;

  for(tot=0; tot<n; tot+=m, off+=m, src+=m){
    bp = s5_ops.bread(ip->dev, s5_iops.bmap(ip, off/BSIZE));
    m = min(n - tot, BSIZE - off%BSIZE);
    memmove(bp->data + off%BSIZE, src, m);
    log_write(bp);
    s5_ops.brelse(bp);
  }

  if(n > 0 && off > ip->size){
    ip->size = off;
    s5_iops.iupdate(ip);
  }
  return n;
}

int
s5_isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(s5_iops.readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}

int
s5_unlink(struct inode *dp, uint off)
{
  struct dirent de;

  memset(&de, 0, sizeof(de));
  if(dp->iops->writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    return -1;

  return 0;
}

int
s5_namecmp(const char *s, const char *t)
{
  return strncmp(s, t, DIRSIZ);
}

int
s5_fill_inode(struct inode *ip) {
  struct s5_inode *s5ip;

  s5ip = alloc_s5_inode();
  if (!s5ip) {
    panic("No s5 inode available");
  }

  ip->i_private = s5ip;

  return 1;
}

struct inode*
s5_iget(uint dev, uint inum)
{
  return iget(dev, inum, &s5_fill_inode);
}

