#ifndef DPRINT_CLANG_FORMAT_WASM_PWD_H
#define DPRINT_CLANG_FORMAT_WASM_PWD_H

struct passwd {
  char* pw_name;
  char* pw_dir;
};

#ifdef __cplusplus
extern "C" {
#endif

struct passwd* getpwuid(unsigned int uid);
int getpwnam_r(const char* name, struct passwd* pwd, char* buffer, unsigned long buffer_size, struct passwd** result);
int getpwuid_r(unsigned int uid, struct passwd* pwd, char* buffer, unsigned long buffer_size, struct passwd** result);

#ifdef __cplusplus
}
#endif

#endif
