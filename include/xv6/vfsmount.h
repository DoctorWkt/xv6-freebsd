// This file implements the Mount table and utilities functions

#include "param.h"
#include "vfs.h"
#include "file.h"
#include "spinlock.h"

#ifndef XV6_VFSMOUNT_H_
#define XV6_VFSMOUNT_H_

#define M_USED 0x1
#define MOUNTSIZE     2   // size of mounted devices

// Mount Table Entry
struct mntentry {
  struct inode *m_inode;
  struct inode *m_rtinode; // Root inode for device
  void *pdata;             // Private date of mountentry. Almost is a superblock
  int dev;                 // Mounted device
  int flag;                // Flag
};

// Mount Table Structure
struct {
  struct spinlock lock;
  struct mntentry mpoint[MOUNTSIZE];
} mtable;

// Utility functions

struct inode* mtablertinode(struct inode * ip);
struct inode* mtablemntinode(struct inode * ip);
int isinoderoot(struct inode* ip);
void mountinit(void);

#endif /* XV6_VFSMOUNT_H_ */

