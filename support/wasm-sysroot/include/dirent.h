#ifndef DPRINT_CLANG_FORMAT_WASM_DIRENT_H
#define DPRINT_CLANG_FORMAT_WASM_DIRENT_H

typedef struct DIR DIR;

struct dirent {
  unsigned long d_ino;
  unsigned char d_type;
  char d_name[256];
};

#ifdef __cplusplus
extern "C" {
#endif

DIR *opendir(const char *path);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);

#ifdef __cplusplus
}
#endif

#endif
