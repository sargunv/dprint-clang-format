#ifndef DPRINT_CLANG_FORMAT_WASM_FCNTL_H
#define DPRINT_CLANG_FORMAT_WASM_FCNTL_H

#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4
#define F_SETLK 6
#define F_SETLKW 7

#define FD_CLOEXEC 1
#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0x0040
#define O_EXCL 0x0080
#define O_TRUNC 0x0200
#define O_APPEND 0x0400
#define O_NONBLOCK 0x0004
#define O_CLOEXEC 0x01000000

struct flock {
  short l_type;
  short l_whence;
  long l_start;
  long l_len;
  int l_pid;
};

#ifdef __cplusplus
extern "C" {
#endif

int fcntl(int fd, int cmd, ...);
int open(const char* path, int flags, ...);

#ifdef __cplusplus
}
#endif

#endif
