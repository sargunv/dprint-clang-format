#ifndef DPRINT_CLANG_FORMAT_WASM_STRING_H
#define DPRINT_CLANG_FORMAT_WASM_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* memchr(const void* ptr, int value, size_t count);
int memcmp(const void* left, const void* right, size_t count);
void* memcpy(void* dest, const void* src, size_t count);
void* memmove(void* dest, const void* src, size_t count);
void* memset(void* dest, int value, size_t count);
char* strchr(const char* str, int value);
char* strcpy(char* dest, const char* src);
int strcmp(const char* left, const char* right);
char* strerror(int errnum);
char* strsignal(int signum);
size_t strlen(const char* str);
size_t strnlen(const char* str, size_t max_len);
int strncmp(const char* left, const char* right, size_t count);
char* strncpy(char* dest, const char* src, size_t count);
char* strdup(const char* str);
char* strtok_r(char* str, const char* delim, char** saveptr);

#ifdef __cplusplus
}
#endif

#endif
