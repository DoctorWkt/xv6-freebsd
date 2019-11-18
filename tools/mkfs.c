#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>

#define dirent xv6dirent // avoid clash with host struct dirent
#define stat   xv6stat   // avoid clash with host struct stat
#include "xv6/types.h"
#include "xv6/vfs.h"
#include "xv6/s5.h"
#include "xv6/stat.h"
#include "xv6/param.h"
#undef dirent
#undef stat

#ifndef static_assert
#define static_assert(a, b) do { switch (0) case 0: case (a): ; } while (0)
#endif

#define NINODES 200
#define HDMAJOR 0

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FSSIZE/(BSIZE*8) + 1;
int ninodeblocks = NINODES / IPB + 1;
int nlog = LOGSIZE;  
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;  // Number of data blocks

int fsfd;
struct s5_superblock sb;
char zeroes[BSIZE];
uint freeinode = 1;
uint freeblock;

void balloc(int);
void wsect(uint, void*);
void winode(uint, struct dinode*);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type, int mtime);
void iappend(uint inum, void *p, int n);
void dappend(int dirino, char *name, int fileino);
void add_directory(int dirino, char *localdir);
int makdir(int dirino, char *newdir, struct stat *sb);

// convert to intel byte order
ushort
xshort(ushort x)
{
  ushort y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  return y;
}

uint
xint(uint x)
{
  uint y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  a[2] = x >> 16;
  a[3] = x >> 24;
  return y;
}

int
main(int argc, char *argv[])
{
  int i, isrootfs = 1, argoff = 0;
  uint rootino, off, devino, hdino;
  struct xv6dirent de;
  char buf[BSIZE];
  struct dinode din;

  //printf("BSIZE: %d\n", BSIZE);
  //printf("sizeof(struct dinode)) %ld\n", sizeof(struct dinode));
  //printf("MAXFILE: %ld\n", MAXFILE);
  //printf("FSSIZE: %d\n", FSSIZE);
  
  static_assert(sizeof(int) == 4, "Integers must be 4 bytes!");

  if(argc < 2){
    fprintf(stderr, "Usage: mkfs [-noroot] fs.img basedir\n");
    exit(1);
  }

  assert((BSIZE % sizeof(struct dinode)) == 0);
  assert((BSIZE % sizeof(struct xv6dirent)) == 0);

  if (strcmp(argv[1], "-noroot") == 0) {
    isrootfs = 0;
    argoff = 1;
  }

  // Open the filesystem image file
  fsfd = open(argv[1 + argoff], O_RDWR|O_CREAT|O_TRUNC, 0666);
  if (fsfd < 0) {
    perror(argv[1 + argoff]);
    exit(1);
  }

  // 1 fs block = 1 disk sector
  // Number of meta blocks: boot block, superblock, log blocks,
  // i-node blocks and the free bitmap blocks
  nmeta = 2 + nlog + ninodeblocks + nbitmap;
  // Now work out how many free blocks are left
  nblocks = FSSIZE - nmeta;

  // Set up the superblock
  sb.size = xint(FSSIZE);
  sb.nblocks = xint(nblocks);
  sb.ninodes = xint(NINODES);
  sb.nlog = xint(nlog);
  sb.logstart = xint(2);
  sb.inodestart = xint(2+nlog);
  sb.bmapstart = xint(2+nlog+ninodeblocks);

  printf("nmeta %d (boot, super, log blocks %u inode blocks %u, bitmap blocks %u) blocks %d total %d\n",
         nmeta, nlog, ninodeblocks, nbitmap, nblocks, FSSIZE);

  freeblock = nmeta;     // The first free block that we can allocate

  // Fill the filesystem with zero'ed blocks
  for(i = 0; i < FSSIZE; i++)
    wsect(i, zeroes);

  // Copy the superblock struct into a zero'ed buf
  // and write it out as block 1
  memset(buf, 0, sizeof(buf));
  memmove(buf, &sb, sizeof(sb) - sizeof(sb.flags));
  wsect(1, buf);

  // Grab an i-node for the root directory
  rootino = ialloc(T_DIR, 0); // Epoch mtime for now
  assert(rootino == ROOTINO);

  // Set up the directory entry for . and add it to the root dir
  // Set up the directory entry for .. and add it to the root dir
  dappend(rootino, ".", rootino);
  dappend(rootino, "..", rootino);

  if (isrootfs) {
    // Create the dev folder
    devino = ialloc(T_DIR, 0);
    dappend(rootino, "dev", devino);
    dappend(devino, ".", devino);
    dappend(devino, "..", devino);

    // Create the device files to access hd
    hdino = ialloc(T_DEV, 0);
    bzero(&de, sizeof(de));
    de.inum = xshort(hdino);
    strcpy(de.name, "hda");
    iappend(devino, &de, sizeof(de));
    rinode(hdino, &din);
    din.major = HDMAJOR;
    din.minor = 0;
    winode(hdino, &din);

    hdino = ialloc(T_DEV, 0);
    bzero(&de, sizeof(de));
    de.inum = xshort(hdino);
    strcpy(de.name, "hdb");
    iappend(devino, &de, sizeof(de));
    rinode(hdino, &din);
    din.major = HDMAJOR;
    din.minor = 1;
    winode(hdino, &din);

    hdino = ialloc(T_DEV, 0);
    bzero(&de, sizeof(de));
    de.inum = xshort(hdino);
    strcpy(de.name, "hdc");
    iappend(devino, &de, sizeof(de));
    rinode(hdino, &din);
    din.major = HDMAJOR;
    din.minor = 2;
    winode(hdino, &din);
  }

  // Add the contents of the command-line directory to the root dir
  add_directory(rootino, argv[2 + argoff]);

  // Fix the size of the root inode dir
  rinode(rootino, &din);
  off = xint(din.size);
  off = ((off/BSIZE) + 1) * BSIZE;
  din.size = xint(off);
  winode(rootino, &din);

  // Mark the in-use blocks in the free block list
  balloc(freeblock);

  exit(0);
}

// Write a sector to the image
void
wsect(uint sec, void *buf)
{
  if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
    perror("lseek");
    exit(1);
  }
  if(write(fsfd, buf, BSIZE) != BSIZE){
    perror("write");
    exit(1);
  }
}

// Write an i-node to the image
void
winode(uint inum, struct dinode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPB);
  *dip = *ip;
  wsect(bn, buf);
}

// Read an i-node from the image
void
rinode(uint inum, struct dinode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPB);
  *ip = *dip;
}

// Read a sector from the image
void
rsect(uint sec, void *buf)
{
  if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
    perror("lseek");
    exit(1);
  }
  if(read(fsfd, buf, BSIZE) != BSIZE){
    perror("read");
    exit(1);
  }
}

// Allocate an i-node
uint
ialloc(ushort type, int mtime)
{
  uint inum = freeinode++;
  struct dinode din;

  assert(freeinode<NINODES);
  bzero(&din, sizeof(din));
  din.type = xshort(type);
  din.nlink = xshort(1);
  din.size = xint(0);
  //XXX din.mtime = mtime;
  winode(inum, &din);
  return inum;
}

// Update the free block list by marking some blocks as in-use
void
balloc(int used)
{
  uchar buf[BSIZE];
  int i;

  //printf("balloc: first %d blocks have been allocated\n", used);
  assert(used < BSIZE*8);
  bzero(buf, BSIZE);
  for(i = 0; i < used; i++){
    buf[i/8] = buf[i/8] | (0x1 << (i%8));
  }
  //printf("balloc: write bitmap block at sector %d\n", sb.bmapstart);
  wsect(sb.bmapstart, buf);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

// Append more data to the file with i-node number inum
void
iappend(uint inum, void *xp, int n)
{
  char *p = (char*)xp;
  uint fbn, off, n1;
  struct dinode din;
  char buf[BSIZE];
  uint indirect[NINDIRECT];
  uint x;

  rinode(inum, &din);
  off = xint(din.size);
  //printf("append inum %d at off %d sz %d\n", inum, off, n);
  while(n > 0){
    fbn = off / BSIZE;
    //printf(" fbn %d\n", fbn);
    assert(fbn < MAXFILE);
    if(fbn < NDIRECT){
      if(xint(din.addrs[fbn]) == 0){
        din.addrs[fbn] = xint(freeblock++);
      }
      x = xint(din.addrs[fbn]);
    } else {
      if(xint(din.addrs[NDIRECT]) == 0){
        din.addrs[NDIRECT] = xint(freeblock++);
      }
      rsect(xint(din.addrs[NDIRECT]), (char*)indirect);
      if(indirect[fbn - NDIRECT] == 0){
        indirect[fbn - NDIRECT] = xint(freeblock++);
        wsect(xint(din.addrs[NDIRECT]), (char*)indirect);
      }
      x = xint(indirect[fbn-NDIRECT]);
    }
    n1 = min(n, (fbn + 1) * BSIZE - off);
    rsect(x, buf);
    bcopy(p, buf + off - (fbn * BSIZE), n1);
    wsect(x, buf);
    n -= n1;
    off += n1;
    p += n1;
  }
  assert(freeblock<FSSIZE);
  din.size = xint(off);
  winode(inum, &din);
}

// Add the given filename and i-number as a directory entry 
void dappend(int dirino, char *name, int fileino)
{
  struct xv6dirent de;

  bzero(&de, sizeof(de));
  de.inum = xshort(fileino);
  strncpy(de.name, name, DIRSIZ);
  iappend(dirino, &de, sizeof(de));
}

// Add a file to the directory with given i-num
void fappend(int dirino, char *filename, struct stat *sb)
{
    char buf[BSIZE];
    int cc, fd, inum;

    // Open the file up
    if((fd = open(filename, 0)) < 0){
      perror(filename);
      exit(1);
    }

    // Allocate an i-node for the file
    inum = ialloc(T_FILE, sb->st_mtime);

    printf(" fappend: %s\n", filename);

    // Add the file's name to the root directory
    dappend(dirino, filename, inum);

    // Read the file's contents in and write to the filesystem
    while((cc = read(fd, buf, sizeof(buf))) > 0)
      iappend(inum, buf, cc);

    close(fd);
}

// Given a local directory name and a directory i-node number
// on the image, add all the files from the local directory
// to the on-image directory
void add_directory(int dirino, char *localdir)
{
  DIR *D;
  struct dirent *dent;
  struct stat sb;
  int newdirino;

  D= opendir(localdir);
  if (D==NULL) {
    perror(localdir);
    exit(1);
  }
  chdir(localdir);

  while ((dent=readdir(D))!=NULL) {

    // Skip . and ..
    if (!strcmp(dent->d_name, ".")) continue;
    if (!strcmp(dent->d_name, "..")) continue;

    if (stat(dent->d_name, &sb)==-1) {
      perror(dent->d_name);
      exit(1);
    }

    if (S_ISDIR(sb.st_mode)) {
      newdirino= makdir(dirino, dent->d_name, &sb);
      add_directory(newdirino, dent->d_name);
    }
    if (S_ISREG(sb.st_mode)) {
      fappend(dirino, dent->d_name, &sb);
    }
  }

  closedir(D);
  chdir("..");
}

// Make a directory entry in the directory with the given i-node number
// and return the new directory's i-number
int makdir(int dirino, char *newdir, struct stat *sb)
{
  int ino;

  // Allocate the inode number for this directory
  // and set up the . and .. entries
  ino = ialloc(T_DIR, sb->st_mtime);
  dappend(ino, ".", ino);
  dappend(ino, "..", dirino);

  // In the parent directory, add the new directory entry
  dappend(dirino, newdir, ino);

  return(ino);
}
