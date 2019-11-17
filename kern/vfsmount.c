#include <xv6/types.h>
#include <xv6/param.h>
#include <xv6/defs.h>
#include <xv6/spinlock.h>
#include <xv6/vfs.h>
#include <xv6/file.h>
#include <xv6/vfsmount.h>

// This function returns the root inode for the mount on inode
struct inode *
mtablertinode(struct inode * ip)
{
  struct inode *rtinode;
  struct mntentry *mp;

  acquire(&mtable.lock);
  for (mp = &mtable.mpoint[0]; mp < &mtable.mpoint[MOUNTSIZE]; mp++) {
    if (mp->m_inode->dev == ip->dev && mp->m_inode->inum == ip->inum) {
      rtinode = mp->m_rtinode;
      release(&mtable.lock);

      return rtinode;
    }
  }
  release(&mtable.lock);

  return 0;
}

// This function returns the mounted on inode for the root inode
struct inode *
mtablemntinode(struct inode * ip)
{
  struct inode *mntinode;
  struct mntentry *mp;

  acquire(&mtable.lock);
  for (mp = &mtable.mpoint[0]; mp < &mtable.mpoint[MOUNTSIZE]; mp++) {
    if (mp->m_rtinode->dev == ip->dev && mp->m_rtinode->inum == ip->inum) {
      mntinode = mp->m_inode;
      release(&mtable.lock);

      return mntinode;
    }
  }
  release(&mtable.lock);

  return 0;
}

int
isinoderoot(struct inode* ip)
{
  struct mntentry *mp;

  acquire(&mtable.lock);
  for (mp = &mtable.mpoint[0]; mp < &mtable.mpoint[MOUNTSIZE]; mp++) {
    if (mp->m_rtinode->dev == ip->dev && mp->m_rtinode->inum == ip->inum) {
      release(&mtable.lock);
      return 1;
    }
  }
  release(&mtable.lock);

  return 0;
}

void
mountinit(void)
{
  initlock(&mtable.lock, "mtable");
}
