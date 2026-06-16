#ifndef DPRINT_CLANG_FORMAT_WASM_SETJMP_H
#define DPRINT_CLANG_FORMAT_WASM_SETJMP_H

typedef int jmp_buf[1];

#define setjmp(env) (0)

#ifdef __cplusplus
extern "C" {
#endif

void longjmp(jmp_buf env, int value) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif
