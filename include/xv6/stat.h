#define T_DIR  0040000   // Directory
#define T_FILE 0100000   // File
#define T_DEV  0020000   // Device

struct xv6stat {
  int type;    // Type of file
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short nlink; // Number of links to file
  uint size;   // Size of file in bytes
};
