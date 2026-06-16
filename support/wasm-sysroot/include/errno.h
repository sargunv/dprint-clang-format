#ifndef DPRINT_CLANG_FORMAT_WASM_ERRNO_H
#define DPRINT_CLANG_FORMAT_WASM_ERRNO_H

#define EDOM 33
#define ERANGE 34
#define EINVAL 22
#define EINTR 4
#define EAGAIN 11
#define EWOULDBLOCK EAGAIN
#define ENOMEM 12
#define EBADF 9
#define EMFILE 24
#define ENOENT 2
#define ESRCH 3
#define EACCES 13
#define EEXIST 17
#define ENOTDIR 20
#define EISDIR 21
#define ENOSYS 38
#define ENOTSUP 95
#define EPIPE 32
#define ETIMEDOUT 110

#ifdef __cplusplus
extern "C" {
#endif

extern int errno;

#ifdef __cplusplus
}
#endif

#endif
