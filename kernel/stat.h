#define T_DIR     1   // Directory
#define T_FILE    2   // File
#define T_DEVICE  3   // Device

//获取文件描述符表示的文件的具体信息
struct stat {
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short type;  // Type of file : T_DIR T_FILE T_DEVICE 
  short nlink; // Number of links to file
  uint64 size; // Size of file in bytes
};
