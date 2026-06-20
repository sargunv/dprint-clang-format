#define HAVE_MMAP 0
#define HAVE_MREMAP 0
#define HAVE_MORECORE 0
#define MORECORE_CANNOT_TRIM 1
#define USE_LOCKS 0
#define NO_MALLINFO 1
#define NO_MALLOC_STATS 1
#define LACKS_FCNTL_H 1
#define LACKS_SYS_PARAM_H 1
#define LACKS_TIME_H 1
#define LACKS_UNISTD_H 1

#include "../support/allocators/dlmalloc/malloc.c"
