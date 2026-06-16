#ifndef DPRINT_CLANG_FORMAT_WASM_ASSERT_H
#define DPRINT_CLANG_FORMAT_WASM_ASSERT_H

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
#define assert(expr) ((expr) ? (void)0 : __builtin_trap())
#endif

#endif
