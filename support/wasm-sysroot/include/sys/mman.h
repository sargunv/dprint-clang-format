#ifndef DPRINT_CLANG_FORMAT_WASM_SYS_MMAN_H
#define DPRINT_CLANG_FORMAT_WASM_SYS_MMAN_H

#include <stddef.h>
#include <sys/types.h>

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_PRIVATE 0x02
#define MAP_SHARED 0x01
#define MAP_ANON 0x20
#define MAP_ANONYMOUS MAP_ANON
#define MAP_FAILED ((void*)-1)

#define MS_SYNC 0x04
#define MADV_DONTNEED 4
#define MADV_WILLNEED 3

#ifdef __cplusplus
extern "C" {
#endif

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void* addr, size_t length);
int mprotect(void* addr, size_t length, int prot);
int msync(void* addr, size_t length, int flags);
int madvise(void* addr, size_t length, int advice);

#ifdef __cplusplus
}
#endif

#endif
