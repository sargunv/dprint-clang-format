#ifndef DPRINT_CLANG_FORMAT_WASM_SYS_STATVFS_H
#define DPRINT_CLANG_FORMAT_WASM_SYS_STATVFS_H

#include <stdint.h>

#define MNT_LOCAL 0x00001000

struct statvfs {
  unsigned long f_bsize;
  unsigned long f_frsize;
  unsigned long f_blocks;
  unsigned long f_bfree;
  unsigned long f_bavail;
  unsigned long f_flags;
  uint64_t f_fsid;
};

#ifdef __cplusplus
extern "C" {
#endif

int statvfs(const char *path, struct statvfs *buffer);
int fstatvfs(int fd, struct statvfs *buffer);

#ifdef __cplusplus
}
#endif

#endif
