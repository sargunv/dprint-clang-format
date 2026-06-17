# Remaining Wasm support audit

This document records what custom support code remains after adopting
[wasm-cxx-shim](https://github.com/zmerlynn/wasm-cxx-shim) v0.5.0, why it
exists, and what might replace it in the future.

## Replaced by wasm-cxx-shim

| Symbol category | Previous location | Now provided by |
|-----------------|-------------------|-----------------|
| `malloc`, `free`, `calloc`, `realloc`, `aligned_alloc` | `wasm_support.cpp` bump allocator | `wasm-cxx-shim::libc` (dlmalloc) |
| `memcpy`, `memmove`, `memset`, `memcmp`, `strlen` | `wasm_support.cpp` | `wasm-cxx-shim::libc` (musl) |
| `operator new` / `delete` (all variants) | `wasm_support.cpp` | `wasm-cxx-shim::libcxx` |
| `__cxa_atexit`, `__cxa_pure_virtual`, `__cxa_throw` (trap) | `wasm_support.cpp` | `wasm-cxx-shim::libcxx` |
| `std::__libcpp_verbose_abort`, `std::exception::~exception` | `wasm_support.cpp` | `wasm-cxx-shim::libcxx` |
| libm (`sin`, `cos`, `pow`, `hypot`, `fma`, …) | not implemented (unused) | `wasm-cxx-shim::libm` (musl) |

Build integration: `scripts/fetch_wasm_cxx_shim.sh`, `scripts/build_wasm_cxx_shim.sh`,
`scripts/wasm_cxx_shim_objects.sh`.

## Remaining custom support

### `src/wasm_support.cpp` (~910 lines)

POSIX, wchar, stdio, and supplemental string/stdlib stubs that
wasm-cxx-shim explicitly does not provide.

| Category | Examples | Why LLVM/Clang needs it | Possible replacement |
|----------|----------|-------------------------|-------------------|
| **POSIX file/process stubs** | `open`, `close`, `read`, `write`, `stat`, `fstat`, `lstat`, `fcntl`, `access`, `unlink`, `mkdir`, `chmod` | LibFormat and LLVM link against Unix APIs; reachable formatting paths do not call them | No packaged zero-import alternative. Keep stubs, or contribute demand-driven stubs upstream to wasm-cxx-shim (out of scope for that project today). |
| **Memory mapping stubs** | `mmap`, `munmap`, `mprotect`, `msync`, `madvise` | LLVM feature detection / unreachable paths | Same as above |
| **Socket stubs** | `socket`, `connect`, `bind`, `listen`, `accept`, `setsockopt` | LLVM networking code paths (unreachable in plugin) | WASI libc (adds imports — incompatible with dprint) |
| **Dynamic linker stubs** | `dlopen`, `dlsym`, `dlclose`, `dlerror`, `dladdr` | `HAVE_DLOPEN` compile-time gates in LLVM | Same as POSIX stubs |
| **Signal/process stubs** | `signal`, `sigaction`, `fork`, `waitpid`, `kill`, `raise` | LLVM signal handling (unreachable) | Same |
| **Time stubs** | `time`, `gettimeofday`, `gmtime_r`, `strftime` | LLVM chrono/time code (mostly stubbed) | wasm-cxx-shim declares types only; implementations stay custom |
| **wchar stubs** | `wcslen`, `wcstol`, `wmemcpy`, `swprintf`, … | libc++ wchar headers / LLVM wide-char paths | openbsd-libc (C only, partial); or musl wchar subset vendored like wasm-cxx-shim does for mem* |
| **stdio stubs** | `printf`, `fprintf`, `snprintf`, `fflush` | LLVM debug/diagnostic paths (return errors) | wasm-cxx-shim ships declarations only; same situation |
| **ctype** | `isalpha`, `tolower`, … | Character classification in LLVM | Could use compiler builtins / header-only; low priority |
| **String/stdlib (extended)** | `strcpy`, `strcmp`, `strtol`, `strdup`, `qsort`, … | LibFormat string handling | [openbsd-libc](https://github.com/trevyn/wasm32-unknown-unknown-openbsd-libc) for real C implementations; wasm-cxx-shim could add musl `str*` on demand |
| **Compiler helper** | `__multi3` (128-bit multiply) | LLVM codegen for wasm32 | `libclang_rt.builtins-wasm32.a` from compiler-rt |
| **setjmp** | `longjmp` → `abort()` | Unreachable in LibFormat; trap if called | Hard to implement without imports; keep trap stub |

### `support/wasm-sysroot/include/` (32 headers)

Declaration-only POSIX/C headers so LLVM/Clang compiles without a real libc.
Implementations live in `wasm_support.cpp`.

**Replacement outlook:** wasm-cxx-shim's minimal headers cover a small libc
subset only. These POSIX headers are still required unless LLVM is patched to
reduce its Unix assumptions. No drop-in replacement.

### `support/libcxx-wasm/include/` (14 headers)

libc++ configuration overlays: disable threads, filesystem, locale; stub
`mutex`, `chrono`, `locale`, stream types.

**Replacement outlook:** LLVM [freestanding libc++ effort](https://github.com/llvm/llvm-project/issues/78350)
may eventually make some overlays unnecessary. Until then, keep them.

### LLVM libc++ source objects (8 `.cpp` files)

Real libc++ implementations compiled from the pinned LLVM tree:
`chrono.cpp`, `error_category.cpp`, `functional.cpp`, `hash.cpp`,
`memory.cpp`, `new_helpers.cpp`, `string.cpp`, `system_error.cpp`.

**Replacement outlook:** wasm-cxx-shim's `libcxx` component is ABI/runtime
stubs only, not a full libc++ build. These objects stay required for LibFormat.

## Import constraint

All artifacts must remain at **import count 0** for dprint's sandboxed host.
This rules out WASI libc, Emscripten, and any library that pulls host syscalls.

## Recommended next steps (if trimming further)

1. Link `libclang_rt.builtins-wasm32.a` and drop `__multi3` from `wasm_support.cpp`.
2. Add stl-probe / libformat-probe to CI after LLVM cross-build.
3. Evaluate openbsd-libc only for the extended `str*`/`stdlib` slice if we want
   real parsers instead of stubs (functional change, not required today).
4. Track wasm-cxx-shim releases; bump `WASM_CXX_SHIM_VERSION` in fetch script.
