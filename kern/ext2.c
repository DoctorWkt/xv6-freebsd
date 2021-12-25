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
#include <xv6/ext2.h>
#include <xv6/find_bits.h>

#define in_range(b, first, len) ((b) >= (first) && (b) <= (first) + (len) - 1)
#define ext2_find_next_zero_bit find_next_zero_bit
#define ext2_test_bit test_bit
#define ext2_set_bit_atomic test_and_set_bit
#define ext2_clear_bit_atomic test_and_clear_bit

static int ext2_block_to_path(struct inode *inode,
                   long i_block, int offsets[4], int *boundary);

static struct ext2_inode * ext2_get_inode(struct superblock *sb,
                                          uint ino, struct buf **bh);

static struct buf * read_block_bitmap(struct superblock *sb,
                                      unsigned int block_group);

static void group_adjust_blocks(struct superblock *sb, int group_no,
                                struct ext2_group_desc *desc, struct buf *bh,
                                int count);

typedef struct {
  uint32 *p;
  uint32 key;
  struct buf *bh;
} Indirect;

static inline void
add_chain(Indirect *p, struct buf *bh, uint32 *v)
{
  p->key = *(p->p = v);
  p->bh = bh;
}

static inline int verify_chain(Indirect *from, Indirect *to)
{
  while (from <= to && from->key == *from->p)
    from++;
  return (from > to);
}


static struct {
  struct spinlock lock;
  struct ext2_inode_info ei[NINODE];
} ext2_ei_pool; // It is a Pool of S5 Superblock Filesystems

struct ext2_inode_info*
alloc_ext2_inode_info()
{
  struct ext2_inode_info *ei;

  acquire(&ext2_ei_pool.lock);
  for (ei = &ext2_ei_pool.ei[0]; ei < &ext2_ei_pool.ei[NINODE]; ei++) {
    if (ei->flags == INODE_FREE) {
      ei->flags |= INODE_USED;
      release(&ext2_ei_pool.lock);

      return ei;
    }
  }
  release(&ext2_ei_pool.lock);

  return 0;
}

static struct {
  struct spinlock lock;
  struct ext2_sb_info sb[MAXVFSSIZE];
} ext2_sb_pool; // It is a Pool of S5 Superblock Filesystems

struct ext2_sb_info*
alloc_ext2_sb()
{
  struct ext2_sb_info *sb;

  acquire(&ext2_sb_pool.lock);
  for (sb = &ext2_sb_pool.sb[0]; sb < &ext2_sb_pool.sb[MAXVFSSIZE]; sb++) {
    if (sb->flags == SB_FREE) {
      sb->flags |= SB_USED;
      release(&ext2_sb_pool.lock);

      return sb;
    }
  }
  release(&ext2_sb_pool.lock);

  return 0;
}

struct vfs_operations ext2_ops = {
  .fs_init = &ext2fs_init,
  .mount   = &ext2_mount,
  .unmount = &ext2_unmount,
  .getroot = &ext2_getroot,
  .readsb  = &ext2_readsb,
  .ialloc  = &ext2_ialloc,
  .balloc  = &ext2_balloc,
  .bzero   = &ext2_bzero,
  .bfree   = &ext2_bfree,
  .brelse  = &brelse,
  .bwrite  = &bwrite,
  .bread   = &bread,
  .namecmp = &ext2_namecmp
};

struct inode_operations ext2_iops = {
  .dirlookup  = &ext2_dirlookup,
  .iupdate    = &ext2_iupdate,
  .itrunc     = &ext2_itrunc,
  .cleanup    = &ext2_cleanup,
  .bmap       = &ext2_bmap,
  .ilock      = &ext2_ilock,
  .iunlock    = &generic_iunlock,
  .stati      = &generic_stati,
  .readi      = &generic_readi,
  .writei     = &ext2_writei,
  .dirlink    = &ext2_dirlink,
  .unlink     = &ext2_unlink,
  .isdirempty = &ext2_isdirempty
};

struct filesystem_type ext2fs = {
  .name = "ext2",
  .ops = &ext2_ops,
  .iops = &ext2_iops
};

int
initext2fs(void)
{
  initlock(&ext2_sb_pool.lock, "ext2_sb_pool");
  /* initlock(&ext2_inode_pool.lock, "ext2_inode_pool"); */
  return register_fs(&ext2fs);
}

int
ext2fs_init(void)
{
  return 0;
}

int
ext2_mount(struct inode *devi, struct inode *ip)
{
  struct mntentry *mp;

  // Read the Superblock
  ext2_ops.readsb(devi->minor, &sb[devi->minor]);

  // Read the root device
  struct inode *devrtip = ext2_ops.getroot(devi->major, devi->minor);

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
ext2_unmount(struct inode *devi)
{
  panic("ext2 unmount op not defined");
  return 0;
}

struct inode *
ext2_getroot(int major, int minor)
{
  return ext2_iget(minor, EXT2_ROOT_INO);
}

static inline int
test_root(int a, int b)
{
  int num = b;

  while (a > num)
    num *= b;
  return num == a;
}

static int
ext2_group_sparse(int group)
{
  if (group <= 1)
    return 1;
  return (test_root(group, 3) || test_root(group, 5) ||
      test_root(group, 7));
}

/**
 *  ext2_bg_has_super - number of blocks used by the superblock in group
 *  @sb: superblock for filesystem
 *  @group: group number to check
 *
 *  Return the number of blocks used by the superblock (primary or backup)
 *  in this group.  Currently this will be only 0 or 1.
 */
int
ext2_bg_has_super(struct superblock *sb, int group)
{
  if (EXT2_HAS_RO_COMPAT_FEATURE(sb, EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER)&&
      !ext2_group_sparse(group))
    return 0;
  return 1;
}

struct ext2_group_desc *
ext2_get_group_desc(struct superblock * sb,
                    unsigned int block_group,
                    struct buf ** bh)
{
  unsigned long group_desc;
  unsigned long offset;
  struct ext2_group_desc * desc;
  struct ext2_sb_info *sbi = EXT2_SB(sb);

  if (block_group >= sbi->s_groups_count) {
    panic("Block group # is too large");
  }

  group_desc = block_group >> EXT2_DESC_PER_BLOCK_BITS(sb);
  offset = block_group & (EXT2_DESC_PER_BLOCK(sb) - 1);
  if (!sbi->s_group_desc[group_desc]) {
    panic("Accessing a group descriptor not loaded");
  }

  desc = (struct ext2_group_desc *) sbi->s_group_desc[group_desc]->data;
  if (bh) {
    *bh = sbi->s_group_desc[group_desc];
  }
  return desc + offset;
}

static unsigned long
descriptor_loc(struct superblock *sb,
               unsigned long logic_sb_block,
               int nr)
{
  unsigned long bg, first_meta_bg;
  int has_super = 0;

  first_meta_bg = EXT2_SB(sb)->s_es->s_first_meta_bg;

  if (!EXT2_HAS_INCOMPAT_FEATURE(sb, EXT2_FEATURE_INCOMPAT_META_BG) ||
      nr < first_meta_bg)
    return (logic_sb_block + nr + 1);
  bg = EXT2_SB(sb)->s_desc_per_block * nr;
  if (ext2_bg_has_super(sb, bg))
    has_super = 1;

  return ext2_group_first_block_no(sb, bg) + has_super;
}

void
ext2_readsb(int dev, struct superblock *sb)
{
  struct buf *bp;
  struct ext2_sb_info *sbi;
  struct ext2_superblock *es;
  uint32 blocksize = EXT2_MIN_BLKSIZE;
  int db_count, i;
  unsigned long block;
  unsigned long logic_sb_block = 1;
  unsigned long offset = 0;

  if((sb->flags & SB_NOT_LOADED) == 0) {
    sbi = alloc_ext2_sb(); // Allocate a new S5 sb struct to the superblock.
  } else{
    sbi = sb->fs_info;
  }

  // These sets are needed because of bread
  sb->major = IDEMAJOR;
  sb->minor = dev;
  sb_set_blocksize(sb, blocksize);
  sb->fs_info = sbi;

  bp = ext2_ops.bread(dev, logic_sb_block); // Read the 1024 bytes starting from the byte 1024
  es = (struct ext2_superblock *)bp->data;

  sbi->s_es = es;
  sbi->s_sbh = bp;
  if (es->s_magic != EXT2_SUPER_MAGIC) {
    ext2_ops.brelse(bp);
    panic("Try to mount a non ext2 fs as an ext2 fs");
  }

  blocksize = EXT2_MIN_BLKSIZE << es->s_log_block_size;

  /* If the blocksize doesn't match, re-read the thing.. */
  if (sb->blocksize != blocksize) {
    ext2_ops.brelse(bp);

    sb_set_blocksize(sb, blocksize);

    logic_sb_block = EXT2_MIN_BLKSIZE / blocksize;
    offset = EXT2_MIN_BLKSIZE % blocksize;
    bp = ext2_ops.bread(dev, logic_sb_block);

    if (!bp) {
      panic("Error on second ext2 superblock read");
    }

    es = (struct ext2_superblock *) (((char *)bp->data) + offset);
    sbi->s_es = es;

    if (es->s_magic != EXT2_SUPER_MAGIC) {
      panic("error: ext2 magic mismatch");
    }
  }

  if (es->s_rev_level == EXT2_GOOD_OLD_REV) {
    sbi->s_inode_size = EXT2_GOOD_OLD_INODE_SIZE;
    sbi->s_first_ino = EXT2_GOOD_OLD_FIRST_INO;
  } else {
    sbi->s_inode_size = es->s_inode_size;
    sbi->s_first_ino = es->s_first_ino;
  }

  sbi->s_blocks_per_group = es->s_blocks_per_group;
  sbi->s_inodes_per_group = es->s_inodes_per_group;

  sbi->s_inodes_per_block = sb->blocksize / sbi->s_inode_size;
  sbi->s_itb_per_group = sbi->s_inodes_per_group / sbi->s_inodes_per_block;
  sbi->s_desc_per_block = sb->blocksize / sizeof(struct ext2_group_desc);

  sbi->s_addr_per_block_bits = ilog2(EXT2_ADDR_PER_BLOCK(sb));
  sbi->s_desc_per_block_bits = ilog2(EXT2_DESC_PER_BLOCK(sb));

  if (sbi->s_blocks_per_group > sb->blocksize * 8) {
    panic("error: #blocks per group too big");
  }

  if (sbi->s_inodes_per_group > sb->blocksize * 8) {
    panic("error: #inodes per group too big");
  }

  sbi->s_groups_count = ((es->s_blocks_count -
                          es->s_first_data_block - 1)
                            / sbi->s_blocks_per_group) + 1;
  db_count = (sbi->s_groups_count + sbi->s_desc_per_block - 1) /
              sbi->s_desc_per_block;

  if (db_count > EXT2_MAX_BGC) {
    panic("error: not enough memory to storage s_group_desc. Consider change the EXT2_MAX_BGC constant");
  }

  /* bgl_lock_init(sbi->s_blockgroup_lock); */

  for (i = 0; i < db_count; i++) {
    block = descriptor_loc(sb, logic_sb_block, i);
    sbi->s_group_desc[i] = ext2_ops.bread(dev, block);
    if (!sbi->s_group_desc[i]) {
      panic("Error on read ext2  group descriptor");
    }
  }

  sbi->s_gdb_count = db_count;
}

/*
 * Read the inode allocation bitmap for a given block_group, reading
 * into the specified slot in the superblock's bitmap cache.
 *
 * Return buffer_head of bitmap on success or NULL.
 */
static struct buf *
read_inode_bitmap(struct superblock * sb, unsigned long block_group)
{
  struct ext2_group_desc *desc;
  struct buf *bh = 0;

  desc = ext2_get_group_desc(sb, block_group, 0);
  if (!desc)
    panic("error on read ext2 inode bitmap");

  bh = ext2_ops.bread(sb->minor, desc->bg_inode_bitmap);
  if (!bh)
    panic("error on read ext2 inode bitmap");
  return bh;
}

/**
 * It is a dummy implementation of ialloc.
 * Current Linux implementation uses an heuristic to alloc inodes
 * in the best place.
 * Our implementation will take an linear search over the inode bitmap
 * and get the first free inode.
 */
struct inode*
ext2_ialloc(uint dev, short type)
{
  int i, group;
  unsigned long ino;
  struct ext2_sb_info *sbi;
  struct buf *bitmap_bh = 0;
  struct buf *bh2;
  struct buf *ibh;
  struct ext2_group_desc *gdp;
  struct ext2_inode *raw_inode;

  sbi = EXT2_SB(&sb[dev]);

  group = 0;
  for(i = 0; i < sbi->s_groups_count; i++) {
    gdp = ext2_get_group_desc(&sb[dev], group, &bh2);

    if (bitmap_bh)
      ext2_ops.brelse(bitmap_bh);

    bitmap_bh = read_inode_bitmap(&sb[dev], group);
    ino = 0;

repeat_in_this_group:
    ino = ext2_find_next_zero_bit((unsigned long *)bitmap_bh->data,
                                  EXT2_INODES_PER_GROUP(&sb[dev]), ino);
    if (ino >= EXT2_INODES_PER_GROUP(&sb[dev])) {
      if (++group == sbi->s_groups_count)
        group = 0;
      continue;
    }
    if (ext2_set_bit_atomic(ino, (unsigned long *)bitmap_bh->data)) {
      /* we lost this inode */
      if (++ino >= EXT2_INODES_PER_GROUP(&sb[dev])) {
        /* this group is exhausted, try next group */
        if (++group == sbi->s_groups_count)
          group = 0;
        continue;
      }
      /* try to find free inode in the same group */
      goto repeat_in_this_group;
    }
    goto got;
  }

  /*
   * Scanned all blockgroups.
   */
  panic("no space to alloc inode");

got:
  ext2_ops.bwrite(bitmap_bh);
  ext2_ops.brelse(bitmap_bh);

  ino += group * EXT2_INODES_PER_GROUP(&sb[dev]) + 1;
  if (ino < EXT2_FIRST_INO(&sb[dev]) || ino > sbi->s_es->s_inodes_count) {
    panic("ext2 invalid inode number allocated");
  }

  /* spin_lock(sb_bgl_lock(sbi, group)); */
  gdp->bg_free_inodes_count -= 1;
  /* spin_unlock(sb_bgl_lock(sbi, group)); */

  ext2_ops.bwrite(bh2);

  raw_inode = ext2_get_inode(&sb[dev], ino, &ibh);

  // Erase the current inode
  memset(raw_inode, 0, sbi->s_inode_size);
  // Translate the xv6 to inode type type
  if (type == T_DIR) {
    raw_inode->i_mode = S_IFDIR;
  } else if (type == T_FILE) {
    raw_inode->i_mode = S_IFREG;
  } else {
    // We did not treat char and block devices with difference.
    panic("ext2: invalid inode mode");
  }

  ext2_ops.bwrite(ibh);
  ext2_ops.brelse(ibh);

  return ext2_iget(dev, ino);
}

uint
ext2_balloc(uint dev)
{
  panic("ext2 balloc op not defined");
}

void
ext2_bzero(int dev, int bno)
{
  panic("ext2 bzero op not defined");
}

void
ext2_bfree(int dev, uint b)
{
  panic("ext2 bfree op not defined");
}

struct inode*
ext2_dirlookup(struct inode *dp, char *name, uint *poff)
{
  uint off, inum, currblk;
  struct ext2_dir_entry_2 *de;
  struct buf *bh;
  int namelen = strlen(name);

  for (off = 0; off < dp->size;) {
    currblk = off / sb[dp->dev].blocksize;

    bh = ext2_ops.bread(dp->dev, ext2_iops.bmap(dp, currblk));

    de = (struct ext2_dir_entry_2 *) (bh->data + (off % sb[dp->dev].blocksize));

    if(de->inode == 0 || de->name_len != namelen) {
      off += de->rec_len;
      ext2_ops.brelse(bh);
      continue;
    }

    if(strncmp(name, de->name, de->name_len) == 0){
      // entry matches path element
      if(poff)
        *poff = off;
      inum = de->inode;
      ext2_ops.brelse(bh);
      return ext2_iget(dp->dev, inum);
    }
    off += de->rec_len;
    ext2_ops.brelse(bh);
  }

  return 0;
}

void
ext2_iupdate(struct inode *ip)
{
  struct buf *bp;
  struct ext2_inode_info *ei;
  struct ext2_inode *raw_inode;

  ei = ip->i_private;
  raw_inode = ext2_get_inode(&sb[ip->dev], ip->inum, &bp);

  raw_inode->i_mode = ei->i_ei.i_mode;
  raw_inode->i_blocks = ei->i_ei.i_blocks;
  raw_inode->i_links_count = ip->nlink;
  memmove(raw_inode->i_block, ei->i_ei.i_block, sizeof(ei->i_ei.i_block));
  raw_inode->i_size = ip->size;

  ext2_ops.bwrite(bp);
  ext2_ops.brelse(bp);
}

/**
 * ext2_free_blocks() -- Free given blocks and update quota and i_blocks
 * @inode:    inode
 * @block:    start physical block to free
 * @count:    number of blocks to free
 */
void
ext2_free_blocks(struct inode * inode, unsigned long block,
                  unsigned long count)
{
  struct buf *bitmap_bh = 0;
  struct buf * bh2;
  unsigned long block_group;
  unsigned long bit;
  unsigned long i;
  unsigned long overflow;
  struct superblock * superb = &sb[inode->dev];
  struct ext2_sb_info * sbi = EXT2_SB(&sb[inode->dev]);
  struct ext2_group_desc * desc;
  struct ext2_superblock * es = sbi->s_es;
  unsigned freed = 0, group_freed;

  if (block < es->s_first_data_block ||
      block + count < block ||
      block + count > es->s_blocks_count) {
    panic("ext2 free blocks in not datazone");
  }

do_more:
  overflow = 0;
  block_group = (block - es->s_first_data_block) / EXT2_BLOCKS_PER_GROUP(superb);
  bit = (block - es->s_first_data_block) % EXT2_BLOCKS_PER_GROUP(superb);
  /*
   * Check to see if we are freeing blocks across a group
   * boundary.
   */
  if (bit + count > EXT2_BLOCKS_PER_GROUP(superb)) {
    overflow = bit + count - EXT2_BLOCKS_PER_GROUP(superb);
    count -= overflow;
  }
  if (bitmap_bh)
    brelse(bitmap_bh);

  bitmap_bh = read_block_bitmap(superb, block_group);
  if (!bitmap_bh)
    goto error_return;

  desc = ext2_get_group_desc(superb, block_group, &bh2);
  if (!desc)
    goto error_return;

  if (in_range (desc->bg_block_bitmap, block, count) ||
      in_range (desc->bg_inode_bitmap, block, count) ||
      in_range (block, desc->bg_inode_table,
                sbi->s_itb_per_group) ||
      in_range (block + count - 1, desc->bg_inode_table,
                sbi->s_itb_per_group)) {
    panic("Freeing blocks on system zone");
    goto error_return;
  }

  for (i = 0, group_freed = 0; i < count; i++) {
    if (!ext2_clear_bit_atomic(bit + i, (unsigned long *)bitmap_bh->data)) {
      panic("ext2 bit already cleared for block");
    } else {
      group_freed++;
    }
  }

  ext2_ops.bwrite(bitmap_bh);
  group_adjust_blocks(superb, block_group, desc, bh2, group_freed);
  freed += group_freed;

  if (overflow) {
    block += count;
    count = overflow;
    goto do_more;
  }
error_return:
  ext2_ops.brelse(bitmap_bh);
}

/**
 * ext2_free_data - free a list of data blocks
 * @inode: inode we are dealing with
 * @p: array of block numbers
 * @q: points immediately past the end of array
 *
 * We are freeing all blocks referred from that array (numbers are
 * stored as little-endian 32-bit) and updating @inode->i_blocks
 * appropriately.
 */
static inline void
ext2_free_data(struct inode *inode, uint32 *p, uint32 *q)
{
  unsigned long block_to_free = 0, count = 0;
  unsigned long nr;

  for ( ; p < q ; p++) {
    nr = *p;
    if (nr) {
      *p = 0;
      /* accumulate blocks to free if they're contiguous */
      if (count == 0)
        goto free_this;
      else if (block_to_free == nr - count)
        count++;
      else {
        ext2_free_blocks(inode, block_to_free, count);
        /* mark_inode_dirty(inode); */
free_this:
        block_to_free = nr;
        count = 1;
      }
    }
  }
  if (count > 0) {
    ext2_free_blocks(inode, block_to_free, count);
    /* mark_inode_dirty(inode); */
  }
}

/**
 * ext2_free_branches - free an array of branches
 * @inode: inode we are dealing with
 * @p: array of block numbers
 * @q: pointer immediately past the end of array
 * @depth: depth of the branches to free
 */
static void
ext2_free_branches(struct inode *inode, uint32 *p, uint32 *q, int depth)
{
  struct buf * bh;
  unsigned long nr;

  if (depth--) {
    int addr_per_block = EXT2_ADDR_PER_BLOCK(&sb[inode->dev]);
    for ( ; p < q ; p++) {
      nr = *p;
      if (!nr)
        continue;
      *p = 0;
      bh = ext2_ops.bread(inode->dev, nr);
      /*
       * A read failure? Report error and clear slot
       * (should be rare).
       */
      if (!bh) {
        panic("ext2 block read failure");
        continue;
      }
      ext2_free_branches(inode,
                         (uint32*)bh->data,
                         (uint32*)bh->data + addr_per_block,
                         depth);
      ext2_ops.brelse(bh);
      ext2_free_blocks(inode, nr, 1);
      /* mark_inode_dirty(inode); */
    }
  } else {
    ext2_free_data(inode, p, q);
  }
}

static void
ext2_release_inode(struct superblock *sb, int group, int dir)
{
  struct ext2_group_desc * desc;
  struct buf *bh;

  desc = ext2_get_group_desc(sb, group, &bh);
  if (!desc) {
    panic("Error on get group descriptor");
    return;
  }

  /* spin_lock(sb_bgl_lock(EXT2_SB(sb), group)); */
  desc->bg_free_inodes_count += 1;
  if (dir)
    desc->bg_used_dirs_count -= 1;
  /* spin_unlock(sb_bgl_lock(EXT2_SB(sb), group)); */
  ext2_ops.bwrite(bh);
}

/*
 * NOTE! When we get the inode, we're the only people
 * that have access to it, and as such there are no
 * race conditions we have to worry about. The inode
 * is not on the hash-lists, and it cannot be reached
 * through the filesystem because the directory entry
 * has been deleted earlier.
 *
 * HOWEVER: we must make sure that we get no aliases,
 * which means that we have to call "clear_inode()"
 * _before_ we mark the inode not in use in the inode
 * bitmaps. Otherwise a newly created file might use
 * the same inode number (not actually the same pointer
 * though), and then we'd have two inodes sharing the
 * same inode number and space on the harddisk.
 */
void
ext2_free_inode (struct inode * inode)
{
  struct superblock *superb = &sb[inode->dev];
  int is_directory;
  unsigned long ino;
  struct buf *bitmap_bh;
  unsigned long block_group;
  unsigned long bit;
  struct ext2_superblock * es;
  struct ext2_inode_info *ei;

  ino = inode->inum;
  ei = inode->i_private;

  es = EXT2_SB(superb)->s_es;
  is_directory = S_ISDIR(ei->i_ei.i_mode);

  if (ino < EXT2_FIRST_INO(superb) ||
      ino > es->s_inodes_count) {
    panic("ext2 reserved or non existent inode");
    return;
  }

  block_group = (ino - 1) / EXT2_INODES_PER_GROUP(superb);
  bit = (ino - 1) % EXT2_INODES_PER_GROUP(superb);
  bitmap_bh = read_inode_bitmap(superb, block_group);
  if (!bitmap_bh)
    return;

  /* Ok, now we can actually update the inode bitmaps.. */
  if (!ext2_clear_bit_atomic(bit, (void *) bitmap_bh->data))
    panic("ext2 bit already cleared");
  else
    ext2_release_inode(superb, block_group, is_directory);

  ext2_ops.bwrite(bitmap_bh);
  ext2_ops.brelse(bitmap_bh);
}

void
ext2_itrunc(struct inode *ip)
{
  uint32 *i_data;
  int offsets[4];
  uint32 nr = 0;
  int n;
  long iblock;
  unsigned blocksize;
  blocksize = sb[ip->dev].blocksize;
  iblock = (blocksize - 1) >> EXT2_BLOCK_SIZE_BITS(&sb[ip->dev]);
  n = ext2_block_to_path(ip, iblock, offsets, 0);

  struct ext2_inode_info *ei = ip->i_private;

  i_data = ei->i_ei.i_block;

  if (n == 0)
    return;

  /* lock block here */

  if (n == 1) {
    ext2_free_data(ip, i_data+offsets[0],
                   i_data + EXT2_NDIR_BLOCKS);
  }

  /* Kill the remaining (whole) subtrees */
  switch (offsets[0]) {
    default:
      nr = i_data[EXT2_IND_BLOCK];
      if (nr) {
        i_data[EXT2_IND_BLOCK] = 0;
        /* mark_inode_dirty(inode); */
        ext2_free_branches(ip, &nr, &nr+1, 1);
      }
    case EXT2_IND_BLOCK:
      nr = i_data[EXT2_DIND_BLOCK];
      if (nr) {
        i_data[EXT2_DIND_BLOCK] = 0;
        /* mark_inode_dirty(inode); */
        ext2_free_branches(ip, &nr, &nr+1, 2);
      }
    case EXT2_DIND_BLOCK:
      nr = i_data[EXT2_TIND_BLOCK];
      if (nr) {
        i_data[EXT2_TIND_BLOCK] = 0;
        /* mark_inode_dirty(inode); */
        ext2_free_branches(ip, &nr, &nr+1, 3);
      }
    case EXT2_TIND_BLOCK:
      ;
  }

  // unlock the inode here
  ext2_free_inode(ip);

  ext2_iops.iupdate(ip);
}

void
ext2_cleanup(struct inode *ip)
{
  memset(ip->i_private, 0, sizeof(struct ext2_inode_info));
}

/**
 * ext2_block_to_path - parse the block number into array of offsets
 * @inode: inode in question (we are only interested in its superblock)
 * @i_block: block number to be parsed
 * @offsets: array to store the offsets in
 * @boundary: set this non-zero if the referred-to block is likely to be
 *             followed (on disk) by an indirect block.
 * To store the locations of file's data ext2 uses a data structure common
 * for UNIX filesystems - tree of pointers anchored in the inode, with
 * data blocks at leaves and indirect blocks in intermediate nodes.
 * This function translates the block number into path in that tree -
 * return value is the path length and @offsets[n] is the offset of
 * pointer to (n+1)th node in the nth one. If @block is out of range
 * (negative or too large) warning is printed and zero returned.
 *
 * Note: function doesn't find node addresses, so no IO is needed. All
 * we need to know is the capacity of indirect blocks (taken from the
 * superblock).
 */

/*
 * Portability note: the last comparison (check that we fit into triple
 * indirect block) is spelled differently, because otherwise on an
 * architecture with 32-bit longs and 8Kb pages we might get into trouble
 * if our filesystem had 8Kb blocks. We might use long long, but that would
 * kill us on x86. Oh, well, at least the sign propagation does not matter -
 * i_block would have to be negative in the very beginning, so we would not
 * get there at all.
 */

static int
ext2_block_to_path(struct inode *inode,
                   long i_block, int offsets[4], int *boundary)
{
  int ptrs = EXT2_ADDR_PER_BLOCK(&sb[inode->dev]);
  int ptrs_bits = EXT2_ADDR_PER_BLOCK_BITS(&sb[inode->dev]);
  const long direct_blocks = EXT2_NDIR_BLOCKS,
        indirect_blocks = ptrs,
        double_blocks = (1 << (ptrs_bits * 2));
  int n = 0;
  int final = 0;

  if (i_block < 0) {
    panic("block_to_path invalid block num");
  } else if (i_block < direct_blocks) {
    offsets[n++] = i_block;
    final = direct_blocks;
  } else if ((i_block -= direct_blocks) < indirect_blocks) {
    offsets[n++] = EXT2_IND_BLOCK;
    offsets[n++] = i_block;
    final = ptrs;
  } else if ((i_block -= indirect_blocks) < double_blocks) {
    offsets[n++] = EXT2_DIND_BLOCK;
    offsets[n++] = i_block >> ptrs_bits;
    offsets[n++] = i_block & (ptrs - 1);
    final = ptrs;
  } else if (((i_block -= double_blocks) >> (ptrs_bits * 2)) < ptrs) {
    offsets[n++] = EXT2_TIND_BLOCK;
    offsets[n++] = i_block >> (ptrs_bits * 2);
    offsets[n++] = (i_block >> ptrs_bits) & (ptrs - 1);
    offsets[n++] = i_block & (ptrs - 1);
    final = ptrs;
  } else {
    panic("This block is out of bounds from this ext2 fs");
  }

  if (boundary)
    *boundary = final - 1 - (i_block & (ptrs - 1));

  return n;
}

static void
ext2_update_branch(struct inode *inode, uint bn, Indirect *chain)
{
  int ptrs = EXT2_ADDR_PER_BLOCK(&sb[inode->dev]);
  int ptrs_bits = EXT2_ADDR_PER_BLOCK_BITS(&sb[inode->dev]);
  const long direct_blocks = EXT2_NDIR_BLOCKS,
        indirect_blocks = ptrs,
        double_blocks = (1 << (ptrs_bits * 2));
  struct ext2_inode_info *ei;

  ei = inode->i_private;

  // Update inode block
  if (bn < 0) {
    panic("block_to_path invalid block num");
  } else if (bn < direct_blocks) {
    if (ei->i_ei.i_block[bn] == 0)
      ei->i_ei.i_block[bn] = chain[0].key;
  } else if ((bn -= direct_blocks) < indirect_blocks) {
    if (ei->i_ei.i_block[EXT2_IND_BLOCK] == 0)
      ei->i_ei.i_block[EXT2_IND_BLOCK] = chain[0].key;
  } else if ((bn -= indirect_blocks) < double_blocks) {
    if (ei->i_ei.i_block[EXT2_DIND_BLOCK] == 0)
      ei->i_ei.i_block[EXT2_DIND_BLOCK] = chain[0].key;
  } else if (((bn -= double_blocks) >> (ptrs_bits * 2)) < ptrs) {
    if (ei->i_ei.i_block[EXT2_TIND_BLOCK] == 0)
      ei->i_ei.i_block[EXT2_TIND_BLOCK] = chain[0].key;
  } else {
    panic("This block is out of bounds from this ext2 fs");
  }

  return;
}

/**
 * ext2_get_branch - read the chain of indirect blocks leading to data
 * @inode: inode in question
 * @depth: depth of the chain (1 - direct pointer, etc.)
 * @offsets: offsets of pointers in inode/indirect blocks
 * @chain: place to store the result
 * @err: here we store the error value
 *
 * Function fills the array of triples <key, p, bh> and returns %NULL
 * if everything went OK or the pointer to the last filled triple
 * (incomplete one) otherwise. Upon the return chain[i].key contains
 * the number of (i+1)-th block in the chain (as it is stored in memory,
 * i.e. little-endian 32-bit), chain[i].p contains the address of that
 * number (it points into struct inode for i==0 and into the bh->b_data
 * for i>0) and chain[i].bh points to the buffer_head of i-th indirect
 * block for i>0 and NULL for i==0. In other words, it holds the block
 * numbers of the chain, addresses they were taken from (and where we can
 * verify that chain did not change) and buffer_heads hosting these
 * numbers.
 *
 * Function stops when it stumbles upon zero pointer (absent block)
 *  (pointer to last triple returned, *@err == 0)
 * or when it gets an IO error reading an indirect block
 *  (ditto, *@err == -EIO)
 * or when it notices that chain had been changed while it was reading
 *  (ditto, *@err == -EAGAIN)
 * or when it reads all @depth-1 indirect blocks successfully and finds
 * the whole chain, all way to the data (returns %NULL, *err == 0).
 */
static Indirect *ext2_get_branch(struct inode *inode,
                                 int depth,
                                 int *offsets,
                                 Indirect chain[4])
{
  Indirect *p = chain;
  struct buf *bh;
  struct ext2_inode_info *ei = inode->i_private;

  add_chain (chain, 0, ei->i_ei.i_block + *offsets);
  if (!p->key)
    goto no_block;
  while (--depth) {
    bh = ext2_ops.bread(inode->dev, p->key);
    if (!bh)
      panic("error on ext2_get_branch");
    if (!verify_chain(chain, p))
      panic("ext2_get_branch chain changed");
    add_chain(++p, bh, (uint32*)bh->data + *++offsets);
    if (!p->key)
      goto no_block;
  }
  return 0;

no_block:
  return p;
}

/**
 * ext2_find_near - find a place for allocation with sufficient locality
 * @inode: owner
 * @ind: descriptor of indirect block.
 *
 * This function returns the preferred place for block allocation.
 * It is used when heuristic for sequential allocation fails.
 * Rules are:
 *   + if there is a block to the left of our position - allocate near it.
 *   + if pointer will live in indirect block - allocate near that block.
 *   + if pointer will live in inode - allocate in the same cylinder group.
 *
 * In the latter case we colour the starting block by the callers PID to
 * prevent it from clashing with concurrent allocations for a different inode
 * in the same block group.   The PID is used here so that functionally related
 * files will be close-by on-disk.
 *
 * Caller must make sure that @ind is valid and will stay that way.
 */

static ext2_fsblk_t ext2_find_near(struct inode *inode, Indirect *ind)
{
  struct ext2_inode_info *ei = inode->i_private;
  uint32 *start = ind->bh ? (uint32 *) ind->bh->data : ei->i_ei.i_block;
  uint32 *p;
  ext2_fsblk_t bg_start;
  ext2_fsblk_t colour;
  ext2_grpblk_t i_block_group;

  /* Try to find previous block */
  for (p = ind->p - 1; p >= start; p--)
    if (*p)
      return *p;

  /* No such thing, so let's try location of indirect block */
  if (ind->bh)
    return ind->bh->blockno;

  /*
   * It is going to be referred from inode itself? OK, just put it into
   * the same cylinder group then.
   */
  i_block_group = (inode->inum - 1) / EXT2_INODES_PER_GROUP(&sb[inode->dev]);
  bg_start = ext2_group_first_block_no(&sb[inode->dev], i_block_group);
  colour = (proc->pid % 16) *
    (EXT2_BLOCKS_PER_GROUP(&sb[inode->dev]) / 16);
  return bg_start + colour;
}

static inline ext2_fsblk_t ext2_find_goal(struct inode *inode, long block,
    Indirect *partial)
{
  return ext2_find_near(inode, partial);
}

/**
 * ext2_blks_to_allocate: Look up the block map and count the number
 * of direct blocks need to be allocated for the given branch.
 *
 * @branch: chain of indirect blocks
 * @k: number of blocks need for indirect blocks
 * @blks: number of data blocks to be mapped.
 * @blocks_to_boundary:  the offset in the indirect block
 *
 * return the total number of blocks to be allocate, including the
 * direct and indirect blocks.
 */
static int
ext2_blks_to_allocate(Indirect * branch, int k, unsigned long blks,
                      int blocks_to_boundary)
{
  unsigned long count = 0;

  /*
   * Simple case, [t,d]Indirect block(s) has not allocated yet
   * then it's clear blocks on that path have not allocated
   */
  if (k > 0) {
    /* right now don't hanel cross boundary allocation */
    if (blks < blocks_to_boundary + 1)
      count += blks;
    else
      count += blocks_to_boundary + 1;
    return count;
  }

  count++;
  while (count < blks && count <= blocks_to_boundary
      && *(branch[0].p + count) == 0) {
    count++;
  }
  return count;
}

/*
 * Read the bitmap for a given block_group,and validate the
 * bits for block/inode/inode tables are set in the bitmaps
 *
 * Return buffer_head on success or NULL in case of failure.
 */
static struct buf *
read_block_bitmap(struct superblock *sb, unsigned int block_group)
{
  struct ext2_group_desc * desc;
  struct buf * bh;
  ext2_fsblk_t bitmap_blk;

  desc = ext2_get_group_desc(sb, block_group, 0);
  if (!desc)
    return 0;
  bitmap_blk = desc->bg_block_bitmap;
  bh = ext2_ops.bread(sb->minor, bitmap_blk);
  if (!bh) {
    return 0;
  }

  /* ext2_valid_block_bitmap(sb, desc, block_group, bh); */
  /*
   * file system mounted not to panic on error, continue with corrupt
   * bitmap
   */
  return bh;
}

/**
 * bitmap_search_next_usable_block()
 * @start:  the starting block (group relative) of the search
 * @bh:   bufferhead contains the block group bitmap
 * @maxblocks:  the ending block (group relative) of the reservation
 *
 * The bitmap search --- search forward through the actual bitmap on disk until
 * we find a bit free.
 */
static ext2_grpblk_t
bitmap_search_next_usable_block(ext2_grpblk_t start, struct buf *bh,
                                ext2_grpblk_t maxblocks)
{
  ext2_grpblk_t next;

  next = ext2_find_next_zero_bit((unsigned long *)bh->data, maxblocks, start);
  if (next >= maxblocks)
    return -1;
  return next;
}

/**
 * find_next_usable_block()
 * @start:  the starting block (group relative) to find next
 *    allocatable block in bitmap.
 * @bh:   bufferhead contains the block group bitmap
 * @maxblocks:  the ending block (group relative) for the search
 *
 * Find an allocatable block in a bitmap.  We perform the "most
 * appropriate allocation" algorithm of looking for a free block near
 * the initial goal; then for a free byte somewhere in the bitmap;
 * then for any free bit in the bitmap.
 */
static ext2_grpblk_t
find_next_usable_block(int start, struct buf *bh, int maxblocks)
{
  ext2_grpblk_t here, next;
  char *p, *r;

  if (start > 0) {
    /*
     * The goal was occupied; search forward for a free
     * block within the next XX blocks.
     *
     * end_goal is more or less random, but it has to be
     * less than EXT2_BLOCKS_PER_GROUP. Aligning up to the
     * next 64-bit boundary is simple..
     */
    ext2_grpblk_t end_goal = (start + 63) & ~63;
    if (end_goal > maxblocks)
      end_goal = maxblocks;
    here = ext2_find_next_zero_bit((unsigned long *)bh->data, end_goal, start);
    if (here < end_goal)
      return here;
  }

  here = start;
  if (here < 0)
    here = 0;

  p = ((char *)bh->data) + (here >> 3);
  r = memscan(p, 0, ((maxblocks + 7) >> 3) - (here >> 3));
  next = (r - ((char *)bh->data)) << 3;

  if (next < maxblocks && next >= here)
    return next;

  here = bitmap_search_next_usable_block(here, bh, maxblocks);
  return here;
}

/**
 * ext2_try_to_allocate()
 * @sb:   superblock
 * @group:  given allocation block group
 * @bitmap_bh:  bufferhead holds the block bitmap
 * @grp_goal:  given target block within the group
 * @count:  target number of blocks to allocate
 * @my_rsv:  reservation window
 *
 * Attempt to allocate blocks within a give range. Set the range of allocation
 * first, then find the first free bit(s) from the bitmap (within the range),
 * and at last, allocate the blocks by claiming the found free bit as allocated.
 *
 * To set the range of this allocation:
 *  if there is a reservation window, only try to allocate block(s)
 *  from the file's own reservation window;
 *  Otherwise, the allocation range starts from the give goal block,
 *  ends at the block group's last block.
 *
 * If we failed to allocate the desired block then we may end up crossing to a
 * new bitmap.
 */
static int
ext2_try_to_allocate(struct superblock *sb, int group,
    struct buf *bitmap_bh, ext2_grpblk_t grp_goal,
    unsigned long *count)
{
  ext2_grpblk_t start, end;
  unsigned long num = 0;

  if (grp_goal > 0)
    start = grp_goal;
  else
    start = 0;
  end = EXT2_BLOCKS_PER_GROUP(sb);

repeat:
  if (grp_goal < 0) {
    grp_goal = find_next_usable_block(start, bitmap_bh, end);
    if (grp_goal < 0)
      goto fail_access;

    int i;

    for (i = 0; i < 7 && grp_goal > start &&
        !ext2_test_bit(grp_goal - 1, (unsigned long *)bitmap_bh->data);
        i++, grp_goal--)
      ;
  }
  start = grp_goal;

  if (ext2_set_bit_atomic(grp_goal,
                          (unsigned long *)bitmap_bh->data)) {
    /*
     * The block was allocated by another thread, or it was
     * allocated and then freed by another thread
     */
    start++;
    grp_goal++;
    if (start >= end)
      goto fail_access;
    goto repeat;
  }
  num++;
  grp_goal++;
  while (num < *count && grp_goal < end &&
         !ext2_set_bit_atomic(grp_goal, (unsigned long*)bitmap_bh->data)) {
    num++;
    grp_goal++;
  }
  *count = num;
  return grp_goal - num;
fail_access:
  *count = num;
  return -1;
}

static void
group_adjust_blocks(struct superblock *sb, int group_no,
                    struct ext2_group_desc *desc, struct buf *bh,
                    int count)
{
  if (count) {
    /* struct ext2_sb_info *sbi = EXT2_SB(sb); */
    unsigned free_blocks;

    /* spin_lock(sb_bgl_lock(sbi, group_no)); */
    free_blocks = desc->bg_free_blocks_count;
    desc->bg_free_blocks_count = free_blocks + count;
    /* spin_unlock(sb_bgl_lock(sbi, group_no)); */
    ext2_ops.bwrite(bh);
  }
}

/*
 * ext2_new_blocks() -- core block(s) allocation function
 * @inode:  file inode
 * @goal:  given target block(filesystem wide)
 * @count:  target number of blocks to allocate
 * @errp:  error code
 *
 * ext2_new_blocks uses a goal block to assist allocation.  If the goal is
 * free, or there is a free block within 32 blocks of the goal, that block
 * is allocated.  Otherwise a forward search is made for a free block; within 
 * each block group the search first looks for an entire free byte in the block
 * bitmap, and then for any free bit if that fails.
 * This function also updates quota and i_blocks field.
 */
ext2_fsblk_t
ext2_new_blocks(struct inode *inode, ext2_fsblk_t goal,
                unsigned long *count, int *errp)
{
  struct buf *bitmap_bh = 0;
  struct buf *gdp_bh;
  int group_no;
  ext2_grpblk_t grp_target_blk; /* blockgroup relative goal block */
  ext2_grpblk_t grp_alloc_blk; /* blockgroup-relative allocated block*/
  ext2_fsblk_t ret_block;  /* filesyetem-wide allocated block */
  int bgi;   /* blockgroup iteration index */
  ext2_grpblk_t free_blocks; /* number of free blocks in a group */
  struct superblock *superb;
  struct ext2_group_desc *gdp;
  struct ext2_superblock *es;
  struct ext2_sb_info *sbi;
  unsigned long ngroups;
  unsigned long num = *count;

  *errp = -1;
  superb = &sb[inode->dev];

  sbi = EXT2_SB(superb);
  es = sbi->s_es;

  /* if (!ext2_has_free_blocks(sbi)) { */
  /*   *errp = -ENOSPC; */
  /*   goto out; */
  /* } */

  /*
   * First, test whether the goal block is free.
   */
  if (goal < es->s_first_data_block ||
      goal >= es->s_blocks_count) {
    goal = es->s_first_data_block;
  }

  group_no = (goal - es->s_first_data_block) / EXT2_BLOCKS_PER_GROUP(superb);
retry_alloc:
  gdp = ext2_get_group_desc(superb, group_no, &gdp_bh);
  if (!gdp)
    goto io_error;

  free_blocks = gdp->bg_free_blocks_count;

  if (free_blocks > 0) {
    grp_target_blk = ((goal - es->s_first_data_block) %
                      EXT2_BLOCKS_PER_GROUP(superb));
    bitmap_bh = read_block_bitmap(superb, group_no);
    if (!bitmap_bh)
      goto io_error;
    grp_alloc_blk = ext2_try_to_allocate(superb, group_no,
                                         bitmap_bh, grp_target_blk, &num);
    if (grp_alloc_blk >= 0)
      goto allocated;
  }

  ngroups = EXT2_SB(superb)->s_groups_count;

  /*
   * Now search the rest of the groups.  We assume that
   * group_no and gdp correctly point to the last group visited.
   */
  for (bgi = 0; bgi < ngroups; bgi++) {
    group_no++;
    if (group_no >= ngroups)
      group_no = 0;
    gdp = ext2_get_group_desc(superb, group_no, &gdp_bh);
    if (!gdp)
      goto io_error;

    free_blocks = gdp->bg_free_blocks_count;
    /*
     * skip this group (and avoid loading bitmap) if there
     * are no free blocks
     */
    if (!free_blocks)
      continue;

    ext2_ops.brelse(bitmap_bh);
    bitmap_bh = read_block_bitmap(superb, group_no);
    if (!bitmap_bh)
      goto io_error;
    /*
     * try to allocate block(s) from this group, without a goal(-1).
     */
    grp_alloc_blk = ext2_try_to_allocate(superb, group_no,
                                         bitmap_bh, -1, &num);
    if (grp_alloc_blk >= 0)
      goto allocated;
  }

  goto out;

allocated:

  ret_block = grp_alloc_blk + ext2_group_first_block_no(superb, group_no);

  if (in_range(gdp->bg_block_bitmap, ret_block, num) ||
      in_range(gdp->bg_inode_bitmap, ret_block, num) ||
      in_range(ret_block, gdp->bg_inode_table,
               EXT2_SB(superb)->s_itb_per_group)         ||
      in_range(ret_block + num - 1, gdp->bg_inode_table,
               EXT2_SB(superb)->s_itb_per_group)) {
    goto retry_alloc;
  }

  if (ret_block + num - 1 >= es->s_blocks_count) {
    panic("Error on ext2 block alloc");
  }

  group_adjust_blocks(superb, group_no, gdp, gdp_bh, -num);

  ext2_ops.bwrite(bitmap_bh);

  *errp = 0;
  ext2_ops.brelse(bitmap_bh);
  /* if (num < *count) { */
  /*   dquot_free_block_nodirty(inode, *count-num); */
  /*   mark_inode_dirty(inode); */
  /*   *count = num; */
  /* } */
  return ret_block;

io_error:
  *errp = -2;
out:
  /*
   * Undo the block allocation
   */
  /* if (!performed_allocation) { */
  /*   dquot_free_block_nodirty(inode, *count); */
  /*   mark_inode_dirty(inode); */
  /* } */
  ext2_ops.brelse(bitmap_bh);
  return 0;
}

/**
 * ext2_alloc_blocks: multiple allocate blocks needed for a branch
 * @indirect_blks: the number of blocks need to allocate for indirect
 *   blocks
 *
 * @new_blocks: on return it will store the new block numbers for
 * the indirect blocks(if needed) and the first direct block,
 * @blks: on return it will store the total number of allocated
 *  direct blocks
 */
static int
ext2_alloc_blocks(struct inode *inode,
                  ext2_fsblk_t goal, int indirect_blks, int blks,
                  ext2_fsblk_t new_blocks[4], int *err)
{
  int target;
  unsigned long count = 0;
  int index = 0;
  ext2_fsblk_t current_block = 0;
  int ret = 0;

  /*
   * Here we try to allocate the requested multiple blocks at once,
   * on a best-effort basis.
   * To build a branch, we should allocate blocks for
   * the indirect blocks(if not allocated yet), and at least
   * the first direct block of this branch.  That's the
   * minimum number of blocks need to allocate(required)
   */
  target = blks + indirect_blks;

  while (1) {
    count = target;
    /* allocating blocks for indirect blocks and direct blocks */
    current_block = ext2_new_blocks(inode,goal,&count,err);
    if (*err)
      goto failed_out;

    target -= count;
    /* allocate blocks for indirect blocks */
    while (index < indirect_blks && count) {
      new_blocks[index++] = current_block++;
      count--;
    }

    if (count > 0)
      break;
  }

  /* save the new block number for the first direct block */
  new_blocks[index] = current_block;

  /* total number of blocks allocated for direct blocks */
  ret = count;
  *err = 0;
  return ret;
failed_out:
  panic("ext2 error on ext2_alloc_blocks");
  return ret;
}

/**
 * ext2_alloc_branch - allocate and set up a chain of blocks.
 * @inode: owner
 * @num: depth of the chain (number of blocks to allocate)
 * @offsets: offsets (in the blocks) to store the pointers to next.
 * @branch: place to store the chain in.
 *
 * This function allocates @num blocks, zeroes out all but the last one,
 * links them into chain and (if we are synchronous) writes them to disk.
 * In other words, it prepares a branch that can be spliced onto the
 * inode. It stores the information about that chain in the branch[], in
 * the same format as ext2_get_branch() would do. We are calling it after
 * we had read the existing part of chain and partial points to the last
 * triple of that (one with zero ->key). Upon the exit we have the same
 * picture as after the successful ext2_get_block(), except that in one
 * place chain is disconnected - *branch->p is still zero (we did not
 * set the last link), but branch->key contains the number that should
 * be placed into *branch->p to fill that gap.
 *
 * If allocation fails we free all blocks we've allocated (and forget
 * their buffer_heads) and return the error value the from failed
 * ext2_alloc_block() (normally -ENOSPC). Otherwise we set the chain
 * as described above and return 0.
 */

static int
ext2_alloc_branch(struct inode *inode,
                  int indirect_blks, int *blks, ext2_fsblk_t goal,
                  int *offsets, Indirect *branch)
{
  int blocksize = sb[inode->dev].blocksize;
  int i, n = 0;
  int err = 0;
  struct buf *bh;
  int num;
  ext2_fsblk_t new_blocks[4];
  ext2_fsblk_t current_block;

  num = ext2_alloc_blocks(inode, goal, indirect_blks,
      *blks, new_blocks, &err);
  if (err)
    return err;

  branch[0].key = new_blocks[0];
  /*
   * metadata blocks and data blocks are allocated.
   */
  for (n = 1; n <= indirect_blks;  n++) {
    /*
     * Get buffer_head for parent block, zero it out
     * and set the pointer to new one, then send;
     * parent to disk.
     */
    bh = ext2_ops.bread(inode->dev, new_blocks[n-1]);
    if (!bh) {
      goto failed;
    }
    branch[n].bh = bh;
    memset(bh->data, 0, blocksize);
    branch[n].p = (uint32 *) bh->data + offsets[n];
    branch[n].key = new_blocks[n];
    *branch[n].p = branch[n].key;
    if ( n == indirect_blks) {
      current_block = new_blocks[n];
      /*
       * End of chain, update the last new metablock of
       * the chain to point to the new allocated
       * data blocks numbers
       */
      for (i=1; i < num; i++)
        *(branch[n].p + i) = ++current_block;
    }
    ext2_ops.bwrite(bh);
  }
  *blks = num;
  return err;

failed:
  panic("ext2 error on allocate blocks branch");
  return err;
}

uint
ext2_bmap(struct inode *ip, uint bn)
{
  /* struct buf *bp; */
  int depth;
  Indirect chain[4];
  Indirect *partial;
  int offsets[4];
  int indirect_blks;
  uint blkn;
  int blocks_to_boundary;
  ext2_fsblk_t goal;
  int count;
  unsigned long maxblocks;
  int err;

  depth = ext2_block_to_path(ip, bn, offsets, &blocks_to_boundary);

  if (depth == 0)
    panic("Wrong depth value");

  partial = ext2_get_branch(ip, depth, offsets, chain);

  if (!partial) {
    goto got_it;
  }

  maxblocks = sb[ip->dev].blocksize >> EXT2_BLOCK_SIZE_BITS(&sb[ip->dev]);

  // The requested block is not allocated yet
  goal = ext2_find_goal(ip, bn, partial);

  /* the number of blocks need to allocate for [d,t]indirect blocks */
  indirect_blks = (chain + depth) - partial - 1;

  indirect_blks = (chain + depth) - partial - 1;
  /*
   * Next look up the indirect map to count the totoal number of
   * direct blocks to allocate for this branch.
   */
  count = ext2_blks_to_allocate(partial, indirect_blks,
      maxblocks, blocks_to_boundary);

  err = ext2_alloc_branch(ip, indirect_blks, &count, goal,
      offsets + (partial - chain), partial);

  if (err < 0)
    panic("error on ext2_alloc_branch");

got_it:
  blkn = chain[depth-1].key;
  ext2_update_branch(ip, bn, chain);

  /* Clean up and exit */
  partial = chain + depth - 1;  /* the whole chain */
/* cleanup: */
  while (partial > chain) {
    brelse(partial->bh);
    partial--;
  }

  return blkn;
}

void
ext2_ilock(struct inode *ip)
{
  struct buf *bp;
  struct ext2_inode *raw_inode;
  struct ext2_inode_info *ei;

  ei = ip->i_private;

  if(ip == 0 || ip->ref < 1)
    panic("ilock");

  acquire(&icache.lock);
  while (ip->flags & I_BUSY)
    sleep(ip, &icache.lock);
  ip->flags |= I_BUSY;
  release(&icache.lock);

  if (!(ip->flags & I_VALID)) {
    raw_inode = ext2_get_inode(&sb[ip->dev], ip->inum, &bp);
    // Translate the inode type to xv6 type
    if (S_ISDIR(raw_inode->i_mode)) {
      ip->type = T_DIR;
    } else if (S_ISREG(raw_inode->i_mode)) {
      ip->type = T_FILE;
    } else if (S_ISCHR(raw_inode->i_mode) || S_ISBLK(raw_inode->i_mode)) {
      ip->type = T_DEV;
    } else {
      panic("ext2: invalid file mode");
    }
    ip->nlink = raw_inode->i_links_count;
    ip->size = raw_inode->i_size;
    memmove(&ei->i_ei, raw_inode, sizeof(ei->i_ei));

    ext2_ops.brelse(bp);
    ip->flags |= I_VALID;
    if (ip->type == 0)
      panic("ext2 ilock: no type");
  }
}

int
ext2_writei(struct inode *ip, char *src, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if (ip->type == T_DEV) {
    if (ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
      return -1;
    return devsw[ip->major].write(ip, src, n);
  }

  if (off > ip->size || off + n < off)
    return -1;

  // TODO: Verify the max file size

  for (tot = 0; tot < n; tot += m, off += m, src += m) {
    bp = ext2_ops.bread(ip->dev, ext2_iops.bmap(ip, off / sb[ip->dev].blocksize));
    m = min(n - tot, sb[ip->dev].blocksize - off % sb[ip->dev].blocksize);
    memmove(bp->data + off % sb[ip->dev].blocksize, src, m);
    ext2_ops.bwrite(bp);
    ext2_ops.brelse(bp);
  }

  if(n > 0 && off > ip->size){
    ip->size = off;
    ext2_iops.iupdate(ip);
  }

  return n;
}

/*
 * Return the offset into page `page_nr' of the last valid
 * byte in that page, plus one.
 */
static unsigned
ext2_last_byte(struct inode *inode, unsigned long page_nr)
{
  unsigned last_byte = inode->size;
  last_byte -= page_nr * sb[inode->dev].blocksize;
  if (last_byte > sb[inode->dev].blocksize)
    last_byte = sb[inode->dev].blocksize;
  return last_byte;
}


int
ext2_dirlink(struct inode *dp, char *name, uint inum, uint type)
{
  int namelen = strlen(name);
  struct buf *bh;
  unsigned chunk_size = sb[dp->dev].blocksize;
  unsigned reclen = EXT2_DIR_REC_LEN(namelen);
  unsigned short rec_len, name_len;
  char *dir_end;
  struct ext2_dir_entry_2 *de;
  int n;
  int numblocks = (dp->size + chunk_size - 1) / chunk_size;
  char *kaddr;

  if (ext2_iops.dirlookup(dp, name, 0) != 0) {
    return -1;
  }

  for (n = 0; n <= numblocks; n++) {
    bh = ext2_ops.bread(dp->dev, ext2_iops.bmap(dp, n));
    kaddr = (char *) bh->data;
    de = (struct ext2_dir_entry_2 *) kaddr;
    dir_end = kaddr + ext2_last_byte(dp, n);
    kaddr += chunk_size - reclen;

    while ((char *)de <= kaddr) {
      if ((char *)de == dir_end) {
        /* We hit i_size */
        name_len = 0;
        rec_len = chunk_size;
        de->rec_len = chunk_size;
        de->inode = 0;
        goto got_it;
      }

      if (de->rec_len == 0) {
        return -1;
      }

      name_len = EXT2_DIR_REC_LEN(de->name_len);
      rec_len = de->rec_len;
      if (!de->inode && rec_len >= reclen)
              goto got_it;
      if (rec_len >= name_len + reclen)
              goto got_it;
      de = (struct ext2_dir_entry_2 *) ((char *) de + rec_len);
    }

    ext2_ops.brelse(bh);
  }

  return -1;

got_it:
  if (de->inode) {
    struct ext2_dir_entry_2 *de1 = (struct ext2_dir_entry_2 *) ((char *) de + name_len);
    de1->rec_len = rec_len - name_len;
    de->rec_len = name_len;
    de = de1;
  }
  de->name_len = namelen;
  strncpy(de->name, name, namelen);
  de->inode = inum;

  // Translate the xv6 to inode type type
  if (type == T_DIR) {
    de->file_type = EXT2_FT_DIR;
  } else if (type == T_FILE) {
    de->file_type = EXT2_FT_REG_FILE;
  } else {
    // We did not treat char and block devices with difference.
    panic("ext2: invalid inode mode");
  }

  ext2_ops.bwrite(bh);
  ext2_ops.brelse(bh);

  if ((n + 1) * chunk_size > dp->size) {
    dp->size += rec_len;
    ext2_iops.iupdate(dp);
  }

  return 0;
}

int
ext2_isdirempty(struct inode *dp)
{
  struct buf *bh;
  unsigned long i;
  char *kaddr;
  struct ext2_dir_entry_2 *de;
  int chunk_size = sb[dp->dev].blocksize;
  int numblocks = (dp->size + chunk_size - 1) / chunk_size;

  for (i = 0; i < numblocks; i++) {
    bh = ext2_ops.bread(dp->dev, ext2_iops.bmap(dp, i));

    if (!bh) {
      panic("ext2_isemptydir error");
    }

    kaddr = (char *)bh->data;
    de = (struct ext2_dir_entry_2 *)kaddr;
    kaddr += ext2_last_byte(dp, i) - EXT2_DIR_REC_LEN(1);

    while ((char *)de <= kaddr) {
      if (de->rec_len == 0) {
        goto not_empty;
      }
      if (de->inode != 0) {
        /* check for . and .. */
        if (de->name[0] != '.')
          goto not_empty;
        if (de->name_len > 2)
          goto not_empty;
        if (de->name_len < 2) {
          if (de->inode != dp->inum)
            goto not_empty;
        } else if (de->name[1] != '.')
          goto not_empty;
      }
      de = (struct ext2_dir_entry_2 *)((char *)de + de->rec_len);
    }
    ext2_ops.brelse(bh);
  }
  return 1;

not_empty:
  ext2_ops.brelse(bh);
  return 0;
}

int
ext2_unlink(struct inode *dp, uint off)
{
  struct buf *bh;
  uint bn, offset;
  struct ext2_dir_entry_2 *dir;
  int chunk_size;

  chunk_size = sb[dp->dev].blocksize;
  bn = off / sb[dp->dev].blocksize;
  offset = off % sb[dp->dev].blocksize;
  bh = ext2_ops.bread(dp->dev, ext2_iops.bmap(dp, bn));

  dir = (struct ext2_dir_entry_2 *)(bh->data + offset);
  char *kaddr = (char *)bh->data;

  unsigned from = ((char*)dir - kaddr) & ~(chunk_size - 1);
  unsigned to = ((char *)dir - kaddr) + dir->rec_len;

  struct ext2_dir_entry_2 *pde = 0;
  struct ext2_dir_entry_2 *de = (struct ext2_dir_entry_2 *) (kaddr + from);

  while ((char*)de < (char*)dir) {
    if (de->rec_len == 0)
      panic("ext2_unlink invalid dir content");
    pde = de;
    de = (struct ext2_dir_entry_2 *)((char *)de + de->rec_len);
  }

  if (pde) {
    from = (char*)pde - (char *)bh->data;
    pde->rec_len = to - from;
  }

  dir->inode = 0;

  ext2_ops.bwrite(bh);
  ext2_ops.brelse(bh);

  return 0;
}

int
ext2_namecmp(const char *s, const char *t)
{
  unsigned short slen = strlen(s), tlen = strlen(t);
  unsigned short size = slen;

  if (slen != tlen)
    return -1;

  if (tlen > slen)
    size = tlen;

  return strncmp(s, t, size);
}

static struct ext2_inode *
ext2_get_inode(struct superblock *sb, uint ino, struct buf **bh)
{
  struct buf * bp;
  unsigned long block_group;
  unsigned long block;
  unsigned long offset;
  struct ext2_group_desc *gdp;
  struct ext2_inode *raw_inode;

  if ((ino != EXT2_ROOT_INO && ino < EXT2_FIRST_INO(sb)) ||
       ino > EXT2_SB(sb)->s_es->s_inodes_count)
    panic("Ext2 invalid inode number");

  block_group = (ino - 1) / EXT2_INODES_PER_GROUP(sb);
  gdp = ext2_get_group_desc(sb, block_group, 0);
  if (!gdp)
    panic("Invalid group descriptor at ext2_get_inode");

  /*
   * Figure out the offset within the block group inode table
   */
  offset = ((ino - 1) % EXT2_INODES_PER_GROUP(sb)) * EXT2_INODE_SIZE(sb);
  block = gdp->bg_inode_table +
    (offset >> EXT2_BLOCK_SIZE_BITS(sb));

  if (!(bp = ext2_ops.bread(sb->minor, block)))
    panic("Error on read the  block inode");

  offset &= (EXT2_BLOCK_SIZE(sb) - 1);
  raw_inode = (struct ext2_inode *)(bp->data + offset);
  if (bh)
    *bh = bp;

  return raw_inode;
}

/**
 * Its is called because the icache lookup failed
 */
int
ext2_fill_inode(struct inode *ip) {
  struct ext2_inode_info *ei;
  struct ext2_inode *raw_inode;
  struct buf *bh;

  ei = alloc_ext2_inode_info();

  if (ei == 0)
    panic("No memory to alloc ext2_inode");

  raw_inode = ext2_get_inode(&sb[ip->dev], ip->inum, &bh);
  memmove(&ei->i_ei, raw_inode, sizeof(ei->i_ei));
  ip->i_private = ei;

  ext2_ops.brelse(bh);

  // Translate the inode type to xv6 type
  if (S_ISDIR(ei->i_ei.i_mode)) {
    ip->type = T_DIR;
  } else if (S_ISREG(ei->i_ei.i_mode)) {
    ip->type = T_FILE;
  } else if (S_ISCHR(ei->i_ei.i_mode) || S_ISBLK(ei->i_ei.i_mode)) {
    ip->type = T_DEV;
  } else {
    panic("ext2: invalid file mode");
  }

  ip->nlink = ei->i_ei.i_links_count;
  ip->size  = ei->i_ei.i_size;
  return 1;
}

struct inode*
ext2_iget(uint dev, uint inum)
{
  return iget(dev, inum, &ext2_fill_inode);
}

