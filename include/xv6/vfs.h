#pragma once

// Simple VFS implementation

#include "types.h"
#include "param.h"
#include "list.h"
#include "stat.h"
#include "spinlock.h"

struct buf;

#define min(a, b) ((a) < (b) ? (a) : (b))

struct superblock {
  int major;        // Driver major number from it superblocks is stored in.
  int minor;        // Driver major number from it superblocks is stored in.
  uint blocksize;  // Block size of this superblock
  void *fs_info;    // Filesystem-specific info
  unsigned char s_blocksize_bits;

  int flags;       // Superblock Falgs to map its usage
};

#define SB_NOT_LOADED 0
#define SB_INITIALIZED 1

#define SB_FREE 0
#define SB_USED 1

struct inode_operations {
  struct inode* (*dirlookup)(struct inode *dp, char *name, uint *off);
  void (*iupdate)(struct inode *ip);
  void (*itrunc)(struct inode *ip);
  void (*cleanup)(struct inode *ip);
  uint (*bmap)(struct inode *ip, uint bn);
  void (*ilock)(struct inode* ip);
  void (*iunlock)(struct inode* ip);
  void (*stati)(struct inode *ip, struct stat *st);
  int (*readi)(struct inode *ip, char *dst, uint off, uint n);
  int (*writei)(struct inode *ip, char *src, uint off, uint n);
  int (*dirlink)(struct inode *dp, char *name, uint inum, uint type);
  int (*unlink)(struct inode *dp, uint off);
  int (*isdirempty)(struct inode *dp);
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// in-memory copy of an inode
struct inode {
  uint dev;                     // Minor Device number
  uint inum;                    // Inode number
  int ref;                      // Reference count
  int flags;                    // I_BUSY, I_VALID
  struct filesystem_type *fs_t; // The Filesystem type this inode is stored in
  struct inode_operations *iops; // The specific inode operations
  void *i_private;               // File System specific informations

  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
};

#define INODE_FREE 0
#define INODE_USED 1

#define I_BUSY 0x1
#define I_VALID 0x2

struct {
  struct spinlock lock;
  struct inode inode[NINODE];
} icache;

// Inode main operations
struct inode* iget(uint dev, uint inum, int (*fill_super)(struct inode *));

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

struct vfs_operations {
  int           (*fs_init)(void);
  int           (*mount)(struct inode *, struct inode *);
  int           (*unmount)(struct inode *);
  struct inode* (*getroot)(int, int);
  void          (*readsb)(int dev, struct superblock *sb);
  struct inode* (*ialloc)(uint dev, short type);
  uint          (*balloc)(uint dev);
  void          (*bzero)(int dev, int bno);
  void          (*bfree)(int dev, uint b);
  void          (*brelse)(struct buf *b);
  void          (*bwrite)(struct buf *b);
  struct buf*   (*bread)(uint dev, uint blockno);
  int           (*namecmp)(const char *s, const char *t);
};

/*
 * This is struct is the map block device and its filesystem.
 * Its main job is return the filesystem type of current (major, minor)
 * mounted device. It is used when it is not possible retrive the
 * filesystem_type from the inode.
 */
struct vfs {
  int major;
  int minor;
  int flag;
  struct filesystem_type *fs_t;
  struct list_head fs_next; // Next mounted on vfs
};
#define VFS_FREE 0
#define VFS_USED 1

struct vfs *rootfs; // It is the golbal pointer to root fs entry

/*
 * This is te representation of mounted lists.
 * It is defferent from the vfssw, because it is mapping the mounted
 * on filesystem per (major, minor)
 */
struct {
  struct spinlock lock;
  struct list_head fs_list;
} vfsmlist;

struct filesystem_type {
  char *name;                     // The filesystem name. Its is used by the mount syscall
  struct vfs_operations *ops;     // VFS operations
  struct inode_operations *iops;  // Pointer to inode operations of this FS.
  struct list_head fs_list;       // This is a list of Filesystems used by vfssw
};

void            installrootfs(void);
void            initvfsmlist(void);
struct vfs*     getvfsentry(int major, int minor);
int             putvfsonlist(int major, int minor, struct filesystem_type *fs_t);
void            initvfssw(void);
int             register_fs(struct filesystem_type *fs);
struct filesystem_type* getfs(const char *fs_name);

// Generic inode operations
void generic_iunlock(struct inode*);
void generic_stati(struct inode *ip, struct stat *st);
int  generic_readi(struct inode *ip, char *dst, uint off, uint n);
int  generic_dirlink(struct inode *dp, char *name, uint inum, uint type);

int sb_set_blocksize(struct superblock *sb, int size);
