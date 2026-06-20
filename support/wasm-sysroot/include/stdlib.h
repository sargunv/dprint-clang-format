#ifndef DPRINT_CLANG_FORMAT_WASM_STDLIB_H
#define DPRINT_CLANG_FORMAT_WASM_STDLIB_H

#include <stddef.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

typedef struct {
  int quot;
  int rem;
} div_t;

typedef struct {
  long quot;
  long rem;
} ldiv_t;

typedef struct {
  long long quot;
  long long rem;
} lldiv_t;

#ifdef __cplusplus
extern "C" {
#endif

int abs(int value);
void abort(void) __attribute__((noreturn));
int atoi(const char *str);
void *calloc(size_t count, size_t size);
div_t div(int numer, int denom);
void exit(int status) __attribute__((noreturn));
void _Exit(int status) __attribute__((noreturn));
void free(void *ptr);
long labs(long value);
ldiv_t ldiv(long numer, long denom);
long long llabs(long long value);
lldiv_t lldiv(long long numer, long long denom);
char *getenv(const char *name);
char *realpath(const char *path, char *resolved_path);
void *malloc(size_t size);
void qsort(void *base, size_t count, size_t size,
           int (*compare)(const void *, const void *));
int rand(void);
void *realloc(void *ptr, size_t size);
void srand(unsigned int seed);
double strtod(const char *str, char **end);
float strtof(const char *str, char **end);
long strtol(const char *str, char **end, int base);
long double strtold(const char *str, char **end);
long long strtoll(const char *str, char **end, int base);
unsigned long strtoul(const char *str, char **end, int base);
unsigned long long strtoull(const char *str, char **end, int base);

#ifdef __cplusplus
}
#endif

#endif
