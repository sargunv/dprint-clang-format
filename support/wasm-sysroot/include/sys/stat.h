#ifndef DPRINT_CLANG_FORMAT_WASM_SYS_STAT_H
#define DPRINT_CLANG_FORMAT_WASM_SYS_STAT_H

#include <sys/types.h>
#include <time.h>

#define S_IFMT 0170000
#define S_IFCHR 0020000
#define S_IFDIR 0040000
#define S_IFBLK 0060000
#define S_IFIFO 0010000
#define S_IFREG 0100000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

#define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec

struct stat {
  dev_t st_dev;
  ino_t st_ino;
  mode_t st_mode;
  unsigned long st_nlink;
  uid_t st_uid;
  gid_t st_gid;
  dev_t st_rdev;
  off_t st_size;
  long st_blksize;
  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
};

#ifdef __cplusplus
extern "C" {
#endif

int fstat(int fd, struct stat* buffer);
int lstat(const char* path, struct stat* buffer);
int mkdir(const char* path, mode_t mode);
int stat(const char* path, struct stat* buffer);
int chmod(const char* path, mode_t mode);
int fchmod(int fd, mode_t mode);
mode_t umask(mode_t mask);

#ifdef __cplusplus
}
#endif

#endif
