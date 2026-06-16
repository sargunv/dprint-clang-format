#ifndef DPRINT_CLANG_FORMAT_WASM_LIMITS_H
#define DPRINT_CLANG_FORMAT_WASM_LIMITS_H

#include_next <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#ifndef _POSIX_ARG_MAX
#define _POSIX_ARG_MAX 4096
#endif

#endif
