#ifndef DPRINT_CLANG_FORMAT_WASM_SYS_UTSNAME_H
#define DPRINT_CLANG_FORMAT_WASM_SYS_UTSNAME_H

struct utsname {
  char sysname[65];
  char nodename[65];
  char release[65];
  char version[65];
  char machine[65];
};

#ifdef __cplusplus
extern "C" {
#endif

int uname(struct utsname* name);

#ifdef __cplusplus
}
#endif

#endif
