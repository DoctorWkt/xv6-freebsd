/* * Virtual File System implementation
 *  This layer is responsible to implement the
 *  abstraction layer over
 * */

#include <xv6/param.h>
#include <xv6/types.h>
#include <xv6/defs.h>
#include <xv6/spinlock.h>
#include <xv6/mmu.h>
#include <xv6/proc.h>
#include <xv6/list.h>
#include <xv6/stat.h>
#include <xv6/file.h>
#include <xv6/buf.h>
#include <xv6/vfs.h>
#include <xv6/device.h>
#include <xv6/vfsmount.h>

#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

/*
 * Its is a pool to allocate vfs structs.
 * We use it becase we don't have a kmalloc function.
 * With an kmalloc implementatios, it need to be removed.
 */
static struct {
  struct spinlock lock;
  struct vfs vfsentry[MAXVFSSIZE];
} vfspool;

struct vfs*
allocvfs()
{
  struct vfs *vfs;

  acquire(&vfspool.lock);
  for (vfs = &vfspool.vfsentry[0]; vfs < &vfspool.vfsentry[MAXVFSSIZE]; vfs++) {
    if (vfs->flag == VFS_FREE) {
      vfs->flag |= VFS_USED;
      release(&vfspool.lock);

      return vfs;
    }
  }
  release(&vfspool.lock);

  return 0;
}

// Add rootvfs on the list
void
installrootfs(void)
{
  if ((rootfs = allocvfs()) == 0) {
    panic("Failed on rootfs allocation");
  }

  rootfs->major = IDEMAJOR;
  rootfs->minor = ROOTDEV;

  struct filesystem_type *fst = getfs(ROOTFSTYPE);
  if (fst == 0) {
    panic("The root fs type is not supported");
  }

  rootfs->fs_t = fst;

  acquire(&vfsmlist.lock);
  list_add_tail(&(rootfs->fs_next), &(vfsmlist.fs_list));
  release(&vfsmlist.lock);
}

void
initvfsmlist(void)
{
  initlock(&vfsmlist.lock, "vfsmlist");
  initlock(&vfspool.lock, "vfspol");
  INIT_LIST_HEAD(&(vfsmlist.fs_list));
}

struct vfs*
getvfsentry(int major, int minor)
{
  struct vfs *vfs;

  list_for_each_entry(vfs, &(vfsmlist.fs_list), fs_next) {
    if (vfs->major == major && vfs->minor == minor) {
      return vfs;
    }
  }

  return 0;
}

int
putvfsonlist(int major, int minor, struct filesystem_type *fs_t)
{
  struct vfs* nvfs;

  if ((nvfs = allocvfs()) == 0) {
    return -1;
  }

  nvfs->major = major;
  nvfs->minor = minor;
  nvfs->fs_t  = fs_t;

  acquire(&vfsmlist.lock);
  list_add_tail(&(nvfs->fs_next), &(vfsmlist.fs_list));
  release(&vfsmlist.lock);

  return 0;
}

struct {
  struct spinlock lock;
  struct list_head fs_list;
} vfssw;

void
initvfssw(void)
{
  initlock(&vfssw.lock, "vfssw");
  INIT_LIST_HEAD(&(vfssw.fs_list));
}

int
register_fs(struct filesystem_type *fs)
{
  acquire(&vfssw.lock);
  list_add(&(fs->fs_list), &(vfssw.fs_list));
  release(&vfssw.lock);

  return 0;
}

struct filesystem_type*
getfs(const char *fs_name)
{
  struct filesystem_type *fs;

  list_for_each_entry(fs, &(vfssw.fs_list), fs_list) {
    if (strcmp(fs_name, fs->name) == 0) {
      return fs;
    }
  }

  return 0;
}

void
generic_iunlock(struct inode *ip)
{
  if(ip == 0 || !(ip->flags & I_BUSY) || ip->ref < 1)
    panic("iunlock");

  acquire(&icache.lock);
  ip->flags &= ~I_BUSY;
  wakeup(ip);
  release(&icache.lock);
}

void
generic_stati(struct inode *ip, struct stat *st)
{
  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->nlink = ip->nlink;
  st->size = ip->size;
}

int
generic_dirlink(struct inode *dp, char *name, uint inum, uint type)
{
  int off;
  struct dirent de;
  struct inode *ip;

  // Check that name is not present.
  if((ip = dp->iops->dirlookup(dp, name, 0)) != 0){
    iput(ip);
    return -1;
  }

  // Look for an empty dirent.
  for(off = 0; off < dp->size; off += sizeof(de)){
    if(dp->iops->readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("dirlink read");
    if(de.inum == 0)
      break;
  }

  strncpy(de.name, name, DIRSIZ);
  de.inum = inum;
  if(dp->iops->writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("dirlink");

  return 0;
}

int
generic_readi(struct inode *ip, char *dst, uint off, uint n)
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
    bp = ip->fs_t->ops->bread(ip->dev, ip->iops->bmap(ip, off/sb[ip->dev].blocksize));
    m = min(n - tot, sb[ip->dev].blocksize - off % sb[ip->dev].blocksize);
    memmove(dst, bp->data + off % sb[ip->dev].blocksize, m);
    ip->fs_t->ops->brelse(bp);
  }

  return n;
}

// Inodes.
//
// An inode describes a single unnamed file.
// The inode disk structure holds metadata: the file's type,
// its size, the number of links referring to it, and the
// list of blocks holding the file's content.
//
// The inodes are laid out sequentially on disk at
// sb.startinode. Each inode has a number, indicating its
// position on the disk.
//
// The kernel keeps a cache of in-use inodes in memory
// to provide a place for synchronizing access
// to inodes used by multiple processes. The cached
// inodes include book-keeping information that is
// not stored on disk: ip->ref and ip->flags.
//
// An inode and its in-memory represtative go through a
// sequence of states before they can be used by the
// rest of the file system code.
//
// * Allocation: an inode is allocated if its type (on disk)
//   is non-zero. ialloc() allocates, iput() frees if
//   the link count has fallen to zero.
//
// * Referencing in cache: an entry in the inode cache
//   is free if ip->ref is zero. Otherwise ip->ref tracks
//   the number of in-memory pointers to the entry (open
//   files and current directories). iget() to find or
//   create a cache entry and increment its ref, iput()
//   to decrement ref.
//
// * Valid: the information (type, size, &c) in an inode
//   cache entry is only correct when the I_VALID bit
//   is set in ip->flags. ilock() reads the inode from
//   the disk and sets I_VALID, while iput() clears
//   I_VALID if ip->ref has fallen to zero.
//
// * Locked: file system code may only examine and modify
//   the information in an inode and its content if it
//   has first locked the inode. The I_BUSY flag indicates
//   that the inode is locked. ilock() sets I_BUSY,
//   while iunlock clears it.
//
// Thus a typical sequence is:
//   ip = iget(dev, inum)
//   ilock(ip)
//   ... examine and modify ip->xxx ...
//   iunlock(ip)
//   iput(ip)
//
// ilock() is separate from iget() so that system calls can
// get a long-term reference to an inode (as for an open file)
// and only lock it for short periods (e.g., in read()).
// The separation also helps avoid deadlock and races during
// pathname lookup. iget() increments ip->ref so that the inode
// stays cached and pointers to it remain valid.
//
// Many internal file system functions expect the caller to
// have locked the inodes involved; this lets callers create
// multi-step atomic operations.

void
iinit(int dev)
{
  initlock(&icache.lock, "icache");
  rootfs->fs_t->ops->readsb(dev, &sb[dev]);
  /* cprintf("sb: size %d nblocks %d ninodes %d nlog %d logstart %d inodestart %d bmap start %d\n", sb[dev].size, */
  /*         sb[dev].nblocks, sb[dev].ninodes, sb[dev].nlog, sb[dev].logstart, sb[dev].inodestart, sb[dev].bmapstart); */
}

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not lock
// the inode and does not read it from disk.
struct inode*
iget(uint dev, uint inum, int (*fill_inode)(struct inode *))
{
  struct inode *ip, *empty;
  struct filesystem_type *fs_t;

  acquire(&icache.lock);

  // Is the inode already cached?
  empty = 0;
  for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
    if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){

      // If the current inode is an mount point
      if (ip->type == T_MOUNT) {
        struct inode *rinode = mtablertinode(ip);

        if (rinode == 0) {
          panic("Invalid Inode on Mount Table");
        }

        rinode->ref++;

        release(&icache.lock);
        return rinode;
      }

      ip->ref++;
      release(&icache.lock);
      return ip;
    }
    if(empty == 0 && ip->ref == 0)    // Remember empty slot.
      empty = ip;
  }

  // Recycle an inode cache entry.
  if(empty == 0)
    panic("iget: no inodes");

  fs_t = getvfsentry(IDEMAJOR, dev)->fs_t;

  ip = empty;
  ip->dev = dev;
  ip->inum = inum;
  ip->ref = 1;
  ip->flags = 0;
  ip->fs_t = fs_t;
  ip->iops = fs_t->iops;

  release(&icache.lock);

  if (!fill_inode(ip)) {
    panic("Error on fill inode");
  }

  return ip;
}

// Increment reference count for ip.
// Returns ip to enable ip = idup(ip1) idiom.
struct inode*
idup(struct inode *ip)
{
  acquire(&icache.lock);
  ip->ref++;
  release(&icache.lock);
  return ip;
}

// Drop a reference to an in-memory inode.
// If that was the last reference, the inode cache entry can
// be recycled.
// If that was the last reference and the inode has no links
// to it, free the inode (and its content) on disk.
// All calls to iput() must be inside a transaction in
// case it has to free the inode.
void
iput(struct inode *ip)
{
  acquire(&icache.lock);
  if(ip->ref == 1 && (ip->flags & I_VALID) && ip->nlink == 0){
    // inode has no links and no other references: truncate and free.
    if(ip->flags & I_BUSY)
      panic("iput busy");
    ip->flags |= I_BUSY;
    release(&icache.lock);
    ip->iops->itrunc(ip);
    ip->type = 0;
    ip->iops->iupdate(ip);
    /* ip->iops->cleanup(ip); */
    acquire(&icache.lock);
    ip->flags = 0;
    wakeup(ip);
  }
  ip->ref--;

  if (ip->ref == 0 ) {
    ip->iops->cleanup(ip);
  }

  release(&icache.lock);
}

// Common idiom: unlock, then put.
void
iunlockput(struct inode *ip)
{
  ip->iops->iunlock(ip);
  iput(ip);
}

// copy stat information from inode.
void
stati(struct inode *ip, struct stat *st)
{
  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->nlink = ip->nlink;
  st->size = ip->size;
}

//PAGEBREAK!
// Paths

// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
static char*
skipelem(char *path, char *name)
{
  char *s;
  int len;

  while(*path == '/')
    path++;
  if(*path == 0)
    return 0;
  s = path;
  while(*path != '/' && *path != 0)
    path++;
  len = path - s;
  if(len >= DIRSIZ)
    memmove(name, s, DIRSIZ);
  else {
    memmove(name, s, len);
    name[len] = 0;
  }
  while(*path == '/')
    path++;
  return path;
}

// Look up and return the inode for a path name.
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes.
// Must be called inside a transaction since it calls iput().
static struct inode*
namex(char *path, int nameiparent, char *name)
{
  struct inode *ip, *next, *ir;

  if(*path == '/')
    ip = rootfs->fs_t->ops->getroot(IDEMAJOR, ROOTDEV);
  else
    ip = idup(proc->cwd);

  while((path = skipelem(path, name)) != 0){
    ip->iops->ilock(ip);
    if(ip->type != T_DIR){
      iunlockput(ip);
      return 0;
    }
    if(nameiparent && *path == '\0'){
      // Stop one level early.
      ip->iops->iunlock(ip);
      return ip;
    }

    component_search:
    if((next = ip->iops->dirlookup(ip, name, 0)) == 0){
      iunlockput(ip);
      return 0;
    }

    ir = next->fs_t->ops->getroot(IDEMAJOR, next->dev);

    if (next->inum == ir->inum  && isinoderoot(ip) && (strncmp(name, "..", 2) == 0)) {
      struct inode *mntinode = mtablemntinode(ip);
      iunlockput(ip);
      ip = mntinode;
      ip->iops->ilock(ip);
      ip->ref++;
      goto component_search;
    }

    iunlockput(ip);

    ip = next;
  }
  if(nameiparent){
    iput(ip);
    return 0;
  }
  return ip;
}

struct inode*
namei(char *path)
{
  char name[DIRSIZ];
  return namex(path, 0, name);
}

struct inode*
nameiparent(char *path, char *name)
{
  return namex(path, 1, name);
}

int
sb_set_blocksize(struct superblock *sb, int size)
{
  /* If we get here, we know size is power of two
   * and it's value is between 512 and PAGE_SIZE */
  sb->blocksize = size;
  sb->s_blocksize_bits = blksize_bits(size);
  return sb->blocksize;
}

