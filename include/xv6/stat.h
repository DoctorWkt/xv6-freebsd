#pragma once

#define T_DIR   1  // Directory
#define T_FILE  2  // File
#define T_DEV   3  // Device
#define T_PIPE  4  // Pipe
#define T_MOUNT 5  // Mount Point

struct stat {
  short type;  // Type of file
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short nlink; // Number of links to file
  int mtime;   // Last modification time
  uint size;   // Size of file in bytes
};
