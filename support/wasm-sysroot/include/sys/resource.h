#ifndef DPRINT_CLANG_FORMAT_WASM_SYS_RESOURCE_H
#define DPRINT_CLANG_FORMAT_WASM_SYS_RESOURCE_H

#include <sys/time.h>

typedef unsigned long rlim_t;

struct rlimit {
  rlim_t rlim_cur;
  rlim_t rlim_max;
};

struct rusage {
  struct timeval ru_utime;
  struct timeval ru_stime;
  long ru_maxrss;
};

#define RLIM_INFINITY ((rlim_t)-1)
#define RLIMIT_CORE 0
#define RLIMIT_DATA 1
#define RLIMIT_STACK 2
#define RLIMIT_RSS 3
#define RUSAGE_SELF 0

#ifdef __cplusplus
extern "C" {
#endif

int getrlimit(int resource, struct rlimit* limit);
int setrlimit(int resource, const struct rlimit* limit);
int getrusage(int who, struct rusage* usage);

#ifdef __cplusplus
}
#endif

#endif
