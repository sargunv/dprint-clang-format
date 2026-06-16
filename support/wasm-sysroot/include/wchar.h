#ifndef DPRINT_CLANG_FORMAT_WASM_WCHAR_H
#define DPRINT_CLANG_FORMAT_WASM_WCHAR_H

#include <stddef.h>

typedef struct {
  unsigned int opaque;
} mbstate_t;

typedef unsigned int wint_t;
#define WEOF ((wint_t)-1)

#ifdef __cplusplus
extern "C" {
#endif

int wmemcmp(const wchar_t* left, const wchar_t* right, size_t count);
wchar_t* wmemcpy(wchar_t* dest, const wchar_t* src, size_t count);
wchar_t* wmemmove(wchar_t* dest, const wchar_t* src, size_t count);
wchar_t* wmemset(wchar_t* dest, wchar_t value, size_t count);
size_t wcslen(const wchar_t* str);
int swprintf(wchar_t* buffer, size_t size, const wchar_t* format, ...);
wchar_t* wcschr(const wchar_t* str, wchar_t value);
wchar_t* wcspbrk(const wchar_t* str, const wchar_t* accept);
wchar_t* wcsrchr(const wchar_t* str, wchar_t value);
wchar_t* wcsstr(const wchar_t* str, const wchar_t* needle);
wchar_t* wmemchr(const wchar_t* ptr, wchar_t value, size_t count);
long wcstol(const wchar_t* str, wchar_t** end, int base);
unsigned long wcstoul(const wchar_t* str, wchar_t** end, int base);
long long wcstoll(const wchar_t* str, wchar_t** end, int base);
unsigned long long wcstoull(const wchar_t* str, wchar_t** end, int base);
float wcstof(const wchar_t* str, wchar_t** end);
double wcstod(const wchar_t* str, wchar_t** end);
long double wcstold(const wchar_t* str, wchar_t** end);

#ifdef __cplusplus
}
#endif

#endif
