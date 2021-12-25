// On-disk file system format for s5 Filesystem.
// Both the kernel and user programs use this header file.

#include <xv6/vfs.h>

#ifndef XV6_S5_H_
#define XV6_S5_H_

#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks | free bit map | data blocks ]
//
// mkfs computes the super block and builds an initial file system. The super describes
// the disk layout:

struct s5_superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  uint inodestart;   // Block number of first inode block
  uint bmapstart;    // Block number of first free map block

  int flags;          // Flag to S5 Superblock.
};

#define S5_SB_FREE 0
#define S5_SB_USED 1

struct s5_inode {
  short type;
  short major;
  short minor;
  short nlink;
  uint size;
  int flag;
  uint addrs[NDIRECT+1];
};

#define S5_INODE_FREE 0
#define S5_INODE_USED 1

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Filesystem specific operations

int            s5fs_init(void);
int            s5_mount(struct inode *, struct inode *);
int            s5_unmount(struct inode *);
struct inode*  s5_getroot();
void           s5_readsb(int dev, struct superblock *sb);
struct inode*  s5_ialloc(uint dev, short type);
uint           s5_balloc(uint dev);
void           s5_bzero(int dev, int bno);
void           s5_bfree(int dev, uint b);
int            s5_namecmp(const char *s, const char *t);
struct inode*  s5_iget(uint dev, uint inum);

// Inode operations of s5 Filesystem
struct inode*  s5_dirlookup(struct inode *dp, char *name, uint *off);
void           s5_iupdate(struct inode *ip);
void           s5_itrunc(struct inode *ip);
void           s5_cleanup(struct inode *ip);
uint           s5_bmap(struct inode *ip, uint bn);
void           s5_ilock(struct inode* ip);
void           s5_iunlock(struct inode* ip);
void           s5_stati(struct inode *ip, struct stat *st);
int            s5_readi(struct inode *ip, char *dst, uint off, uint n);
int            s5_writei(struct inode *ip, char *src, uint off, uint n);
int            s5_dirlink(struct inode *dp, char *name, uint inum);
int            s5_unlink(struct inode *dp, uint off);
int            s5_isdirempty(struct inode *dp);

#endif /* XV6_S5_h */

