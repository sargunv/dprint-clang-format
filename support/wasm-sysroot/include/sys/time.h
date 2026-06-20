#ifndef DPRINT_CLANG_FORMAT_WASM_SYS_TIME_H
#define DPRINT_CLANG_FORMAT_WASM_SYS_TIME_H

#include <time.h>

struct timeval {
  time_t tv_sec;
  long tv_usec;
};

#ifdef __cplusplus
extern "C" {
#endif

int gettimeofday(struct timeval *tv, void *tz);

#ifdef __cplusplus
}
#endif

#endif
