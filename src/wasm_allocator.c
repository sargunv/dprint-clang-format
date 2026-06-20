#define HAVE_MMAP 0
#define HAVE_MREMAP 0
#define HAVE_MORECORE 1
#define MORECORE wasm_morecore
#define MORECORE_CONTIGUOUS 1
#define MORECORE_CANNOT_TRIM 1
#define USE_LOCKS 0
#define NO_MALLINFO 1
#define NO_MALLOC_STATS 1
#define LACKS_FCNTL_H 1
#define LACKS_SYS_PARAM_H 1
#define LACKS_TIME_H 1
#define LACKS_UNISTD_H 1

#include <stddef.h>
#include <stdint.h>

extern char __heap_end;

static char *wasm_heap_end = &__heap_end;

static void *wasm_morecore(ptrdiff_t size) {
  static const uintptr_t page_size = 65536;

  if (size == 0) {
    return wasm_heap_end;
  }
  if (size < 0) {
    return (void *)-1;
  }

  char *old_end = wasm_heap_end;
  if ((uintptr_t)old_end > UINTPTR_MAX - (uintptr_t)size) {
    return (void *)-1;
  }

  uintptr_t requested_end = (uintptr_t)old_end + (uintptr_t)size;
  uintptr_t current_pages = __builtin_wasm_memory_size(0);
  uintptr_t current_end = current_pages * page_size;
  if (requested_end > current_end) {
    uintptr_t required_pages = (requested_end + page_size - 1) / page_size;
    uintptr_t grow_pages = required_pages - current_pages;
    if (__builtin_wasm_memory_grow(0, grow_pages) == (size_t)-1) {
      return (void *)-1;
    }
  }

  wasm_heap_end = (char *)requested_end;
  return old_end;
}

#include "../support/allocators/dlmalloc/malloc.c"
