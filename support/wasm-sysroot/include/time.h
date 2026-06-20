#ifndef DPRINT_CLANG_FORMAT_WASM_TIME_H
#define DPRINT_CLANG_FORMAT_WASM_TIME_H

typedef long time_t;

#ifndef DPRINT_CLANG_FORMAT_WASM_TIMESPEC
#define DPRINT_CLANG_FORMAT_WASM_TIMESPEC
struct timespec {
  time_t tv_sec;
  long tv_nsec;
};
#endif

struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
};

#ifdef __cplusplus
extern "C" {
#endif

struct tm *gmtime_r(const time_t *timep, struct tm *result);
struct tm *localtime_r(const time_t *timep, struct tm *result);
struct tm *gmtime(const time_t *timep);
struct tm *localtime(const time_t *timep);
unsigned long strftime(char *buffer, unsigned long size, const char *format,
                       const struct tm *timeptr);
time_t time(time_t *out);

#ifdef __cplusplus
}
#endif

#endif
