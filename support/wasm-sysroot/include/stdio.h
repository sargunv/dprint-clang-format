#ifndef DPRINT_CLANG_FORMAT_WASM_STDIO_H
#define DPRINT_CLANG_FORMAT_WASM_STDIO_H

#define EOF (-1)
#define BUFSIZ 1024

typedef struct FILE FILE;

#ifdef __cplusplus
extern "C" {
#endif

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int remove(const char* path);
int rename(const char* old_path, const char* new_path);
int feof(FILE* stream);
int ferror(FILE* stream);
int fflush(FILE* stream);
int fprintf(FILE* stream, const char* format, ...);
int printf(const char* format, ...);
int snprintf(char* buffer, unsigned long size, const char* format, ...);
int vsnprintf(char* buffer, unsigned long size, const char* format, __builtin_va_list args);

#ifdef __cplusplus
}
#endif

#endif
