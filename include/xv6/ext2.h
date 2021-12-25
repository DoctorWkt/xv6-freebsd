#pragma once

/**
 * This is the ext2 implementation
 * It is based on Linux ext2 implementation
 **/

#include <xv6/types.h>
#include <xv6/vfs.h>

/* data type for block offset of block group */
typedef int ext2_grpblk_t;

/* data type for filesystem-wide blocks number */
typedef unsigned long ext2_fsblk_t;

#define EXT2_MIN_BLKSIZE 1024
#define EXT2_SUPER_MAGIC 0xEF53

#define EXT2_MAX_BGC 40

#define EXT2_NAME_LEN 255

/**
 * EXT2_DIR_PAD defines the directory entries boundaries
 *
 * NOTE: It must be a multiple of 4
 */
#define EXT2_DIR_PAD                    4
#define EXT2_DIR_ROUND                  (EXT2_DIR_PAD - 1)
#define EXT2_DIR_REC_LEN(name_len)      (((name_len) + 8 + EXT2_DIR_ROUND) & \
                                         ~EXT2_DIR_ROUND)
#define EXT2_MAX_REC_LEN                ((1<<16)-1)

/**
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
enum {
  EXT2_FT_UNKNOWN  = 0,
  EXT2_FT_REG_FILE = 1,
  EXT2_FT_DIR      = 2,
  EXT2_FT_CHRDEV   = 3,
  EXT2_FT_BLKDEV   = 4,
  EXT2_FT_FIFO     = 5,
  EXT2_FT_SOCK     = 6,
  EXT2_FT_SYMLINK  = 7,
  EXT2_FT_MAX
};

/*
 * second extended-fs super-block data in memory
 */
struct ext2_sb_info {
  unsigned long s_inodes_per_block; /* Number of inodes per block */
  unsigned long s_blocks_per_group; /* Number of blocks in a group */
  unsigned long s_inodes_per_group; /* Number of inodes in a group */
  unsigned long s_itb_per_group;    /* Number of inode table blocks per group */
  unsigned long s_gdb_count;        /* Number of group descriptor blocks */
  unsigned long s_desc_per_block;   /* Number of group descriptors per block */
  unsigned long s_groups_count;     /* Number of groups in the fs */
  unsigned long s_overhead_last;    /* Last calculated overhead */
  unsigned long s_blocks_last;      /* Last seen block count */
  struct buf *s_sbh;                /* Buffer containing the super block */
  struct ext2_superblock *s_es;     /* Pointer to the super block in the buffer */
  struct buf *s_group_desc[EXT2_MAX_BGC];
  unsigned long  s_sb_block;
  unsigned short s_pad;
  int s_addr_per_block_bits;
  int s_desc_per_block_bits;
  int s_inode_size;
  int s_first_ino;
  unsigned long s_dir_count;
  uint8 *s_debts;
  int flags;
};

static inline struct ext2_sb_info *
EXT2_SB(struct superblock *sb)
{
  return sb->fs_info;
}

/*
 * Macro-instructions used to manage several block sizes
 */
#define EXT2_MIN_BLOCK_SIZE    1024
#define EXT2_MAX_BLOCK_SIZE         4096
#define EXT2_BLOCK_SIZE(s)          ((s)->blocksize)
#define EXT2_ADDR_PER_BLOCK(s)      (EXT2_BLOCK_SIZE(s) / sizeof (uint32))
#define EXT2_BLOCK_SIZE_BITS(s)     ((s)->s_blocksize_bits)
#define EXT2_ADDR_PER_BLOCK_BITS(s) (EXT2_SB(s)->s_addr_per_block_bits)
#define EXT2_INODE_SIZE(s)          (EXT2_SB(s)->s_inode_size)
#define EXT2_FIRST_INO(s)           (EXT2_SB(s)->s_first_ino)

/**
 * This struct is based on the Linux Sorce Code fs/ext2/ext2.h.
 * It is the ext2 superblock layout definition.
 */
struct ext2_superblock {
  uint32 s_inodes_count;    /* Inodes count */
  uint32 s_blocks_count;    /* Blocks count */
  uint32 s_r_blocks_count;  /* Reserved blocks count */
  uint32 s_free_blocks_count;  /* Free blocks count */
  uint32 s_free_inodes_count;  /* Free inodes count */
  uint32 s_first_data_block;  /* First Data Block */
  uint32 s_log_block_size;  /* Block size */
  uint32 s_log_frag_size;  /* Fragment size */
  uint32 s_blocks_per_group;  /* # Blocks per group */
  uint32 s_frags_per_group;  /* # Fragments per group */
  uint32 s_inodes_per_group;  /* # Inodes per group */
  uint32 s_mtime;    /* Mount time */
  uint32 s_wtime;    /* Write time */
  uint16 s_mnt_count;    /* Mount count */
  uint16 s_max_mnt_count;  /* Maximal mount count */
  uint16 s_magic;    /* Magic signature */
  uint16 s_state;    /* File system state */
  uint16 s_errors;    /* Behaviour when detecting errors */
  uint16 s_minor_rev_level;   /* minor revision level */
  uint32 s_lastcheck;    /* time of last check */
  uint32 s_checkinterval;  /* max. time between checks */
  uint32 s_creator_os;    /* OS */
  uint32 s_rev_level;    /* Revision level */
  uint16 s_def_resuid;    /* Default uid for reserved blocks */
  uint16 s_def_resgid;    /* Default gid for reserved blocks */

  /*
   * These fields are for EXT2_DYNAMIC_REV superblocks only.
   *
   * Note: the difference between the compatible feature set and
   * the incompatible feature set is that if there is a bit set
   * in the incompatible feature set that the kernel doesn't
   * know about, it should refuse to mount the filesystem.
   *
   * e2fsck's requirements are more strict; if it doesn't know
   * about a feature in either the compatible or incompatible
   * feature set, it must abort and not try to meddle with
   * things it doesn't understand...
   */
  uint32 s_first_ino;     /* First non-reserved inode */
  uint16 s_inode_size;     /* size of inode structure */
  uint16 s_block_group_nr;   /* block group # of this superblock */
  uint32 s_feature_compat;   /* compatible feature set */
  uint32 s_feature_incompat;   /* incompatible feature set */
  uint32 s_feature_ro_compat;   /* readonly-compatible feature set */
  uint8  s_uuid[16];    /* 128-bit uuid for volume */
  char   s_volume_name[16];   /* volume name */
  char   s_last_mounted[64];   /* directory where last mounted */
  uint32 s_algorithm_usage_bitmap; /* For compression */

  /*
   * Performance hints.  Directory preallocation should only
   * happen if the EXT2_COMPAT_PREALLOC flag is on.
   */
  uint8  s_prealloc_blocks;  /* Nr of blocks to try to preallocate*/
  uint8  s_prealloc_dir_blocks;  /* Nr to preallocate for dirs */
  uint16 s_padding1;

  /*
   * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
   */
  uint8  s_journal_uuid[16];  /* uuid of journal superblock */
  uint32 s_journal_inum;    /* inode number of journal file */
  uint32 s_journal_dev;    /* device number of journal file */
  uint32 s_last_orphan;    /* start of list of inodes to delete */
  uint32 s_hash_seed[4];    /* HTREE hash seed */
  uint8  s_def_hash_version;  /* Default hash version to use */
  uint8  s_reserved_char_pad;
  uint16 s_reserved_word_pad;
  uint32 s_default_mount_opts;
  uint32 s_first_meta_bg;   /* First metablock block group */
  uint32 s_reserved[190];  /* Padding to the end of the block */
};

#define EXT2_NDIR_BLOCKS  12
#define EXT2_IND_BLOCK    EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK   (EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK   (EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS     (EXT2_TIND_BLOCK + 1)

/*
 * Structure of an inode on the disk
 */
struct ext2_inode {
  uint16 i_mode;  /* File mode */
  uint16 i_uid;   /* Low 16 bits of Owner Uid */
  uint32 i_size;  /* Size in bytes */
  uint32 i_atime; /* Access time */
  uint32 i_ctime; /* Creation time */
  uint32 i_mtime; /* Modification time */
  uint32 i_dtime; /* Deletion Time */
  uint16 i_gid;   /* Low 16 bits of Group Id */
  uint16 i_links_count; /* Links count */
  uint32 i_blocks; /* Blocks count */
  uint32 i_flags;  /* File flags */
  union {
    struct {
      uint32  l_i_reserved1;
    } linux1;
    struct {
      uint32  h_i_translator;
    } hurd1;
    struct {
      uint32  m_i_reserved1;
    } masix1;
  } osd1;    /* OS dependent 1 */
  uint32 i_block[EXT2_N_BLOCKS];  /* Pointers to blocks */
  uint32 i_generation;  /* File version (for NFS) */
  uint32 i_file_acl;    /* File ACL */
  uint32 i_dir_acl;     /* Directory ACL */
  uint32 i_faddr;       /* Fragment address */
  union {
    struct {
      uint8  l_i_frag;  /* Fragment number */
      uint8  l_i_fsize; /* Fragment size */
      uint16 i_pad1;
      uint16 l_i_uid_high;  /* these 2 fields    */
      uint16 l_i_gid_high;  /* were reserved2[0] */
      uint32 l_i_reserved2;
    } linux2;
    struct {
      uint8  h_i_frag;  /* Fragment number */
      uint8  h_i_fsize; /* Fragment size */
      uint16 h_i_mode_high;
      uint16 h_i_uid_high;
      uint16 h_i_gid_high;
      uint32 h_i_author;
    } hurd2;
    struct {
      uint8  m_i_frag;  /* Fragment number */
      uint8  m_i_fsize; /* Fragment size */
      uint16 m_pad1;
      uint32 m_i_reserved2[2];
    } masix2;
  } osd2;   /* OS dependent 2 */
};

struct ext2_inode_info {
  struct ext2_inode i_ei;
  uint flags;
};

#define EXT2_ROOT_INO  2  /* Root inode */

/*
 * Structure of a directory entry
 */

struct ext2_dir_entry {
  uint32 inode;       /* Inode number */
  uint16 rec_len;     /* Directory entry length */
  uint16 name_len;    /* Name length */
  char   name[];      /* File name, up to EXT2_NAME_LEN */
};

/*
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext2_dir_entry_2 {
  uint32 inode;      /* Inode number */
  uint16 rec_len;    /* Directory entry length */
  uint8  name_len;   /* Name length */
  uint8  file_type;
  char   name[];     /* File name, up to EXT2_NAME_LEN */
};

/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
  uint32 bg_block_bitmap;       /* Blocks bitmap block */
  uint32 bg_inode_bitmap;       /* Inodes bitmap block */
  uint32 bg_inode_table;        /* Inodes table block */
  uint16 bg_free_blocks_count;  /* Free blocks count */
  uint16 bg_free_inodes_count;  /* Free inodes count */
  uint16 bg_used_dirs_count;    /* Directories count */
  uint16 bg_pad;
  uint32 bg_reserved[3];
};

/*
* Macro-instructions used to manage group descriptors
*/
#define EXT2_BLOCKS_PER_GROUP(s)  (EXT2_SB(s)->s_blocks_per_group)
#define EXT2_DESC_PER_BLOCK(s)    (EXT2_SB(s)->s_desc_per_block)
#define EXT2_INODES_PER_GROUP(s)  (EXT2_SB(s)->s_inodes_per_group)
#define EXT2_DESC_PER_BLOCK_BITS(s) (EXT2_SB(s)->s_desc_per_block_bits)

/*
 * Codes for operating systems
 */
#define EXT2_OS_LINUX    0
#define EXT2_OS_HURD     1
#define EXT2_OS_MASIX    2
#define EXT2_OS_FREEBSD  3
#define EXT2_OS_LITES    4

/*
 * Revision levels
 */
#define EXT2_GOOD_OLD_REV 0  /* The good old (original) format */
#define EXT2_DYNAMIC_REV  1  /* V2 format w/ dynamic inode sizes */

#define EXT2_CURRENT_REV EXT2_GOOD_OLD_REV
#define EXT2_MAX_SUPP_REV EXT2_DYNAMIC_REV

#define EXT2_GOOD_OLD_INODE_SIZE 128

/*
 * Special inode numbers
 */
#define EXT2_BAD_INO          1  /* Bad blocks inode */
#define EXT2_ROOT_INO         2  /* Root inode */
#define EXT2_BOOT_LOADER_INO  5  /* Boot loader inode */
#define EXT2_UNDEL_DIR_INO    6  /* Undelete directory inode */

/* First non-reserved inode for old ext2 filesystems */
#define EXT2_GOOD_OLD_FIRST_INO 11

#define EXT2_HAS_INCOMPAT_FEATURE(sb,mask)     \
  ( EXT2_SB(sb)->s_es->s_feature_incompat & mask )
#define EXT2_HAS_RO_COMPAT_FEATURE(sb,mask)    \
  ( EXT2_SB(sb)->s_es->s_feature_ro_compat & mask )

#define EXT2_FEATURE_INCOMPAT_META_BG   0x0010
#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001

static inline ext2_fsblk_t
ext2_group_first_block_no(struct superblock *sb, unsigned long group_no)
{
  return group_no * (ext2_fsblk_t)EXT2_BLOCKS_PER_GROUP(sb) +
          EXT2_SB(sb)->s_es->s_first_data_block;
}

// Filesystem specific operations

int            ext2fs_init(void);
int            ext2_mount(struct inode *, struct inode *);
int            ext2_unmount(struct inode *);
struct inode*  ext2_getroot();
void           ext2_readsb(int dev, struct superblock *sb);
struct inode*  ext2_ialloc(uint dev, short type);
uint           ext2_balloc(uint dev);
void           ext2_bzero(int dev, int bno);
void           ext2_bfree(int dev, uint b);
int            ext2_namecmp(const char *s, const char *t);
struct inode*  ext2_iget(uint dev, uint inum);

// Inode operations of ext2 Filesystem
struct inode*  ext2_dirlookup(struct inode *dp, char *name, uint *off);
void           ext2_iupdate(struct inode *ip);
void           ext2_itrunc(struct inode *ip);
void           ext2_cleanup(struct inode *ip);
uint           ext2_bmap(struct inode *ip, uint bn);
void           ext2_ilock(struct inode* ip);
void           ext2_iunlock(struct inode* ip);
void           ext2_stati(struct inode *ip, struct stat *st);
int            ext2_readi(struct inode *ip, char *dst, uint off, uint n);
int            ext2_writei(struct inode *ip, char *src, uint off, uint n);
int            ext2_dirlink(struct inode *dp, char *name, uint inum, uint type);
int            ext2_unlink(struct inode *dp, uint off);
int            ext2_isdirempty(struct inode *dp);

#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFDIR 0x4000

// Stat operations

#define S_IFMT  00170000
#define S_IFSOCK 0140000
#define S_IFLNK  0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000

#define S_ISLNK(m)     (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)     (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)     (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)     (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)     (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)    (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)    (((m) & S_IFMT) == S_IFSOCK)

#define S_IR  WXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001
