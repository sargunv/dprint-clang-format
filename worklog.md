# Worklog: simplify libformat Wasm dprint plugin

Goal: keep the repo maintainable with one zero-import Wasm dprint plugin backed by a
minimal LibFormat link closure. Wasm byte size is secondary; fewer scripts, probes, and
support files matter more.

## Current state

- `pixi run smoke-dprint-plugin` passes: zero Wasm imports, ~3.0 MiB plugin.
- Scripts: `fetch-llvm` → `configure-llvm-wasm` → `link_libformat_wasm.sh` → smoke.
- Runtime split: `wasm_runtime_core.cpp` (allocator, mem*, C++ operators) +
  `wasm_posix_stubs.cpp` (POSIX stubs required at link time).
- LLVM patch `support/patches/dprint-wasm-no-real-fs.patch` applied by `fetch-llvm`.

## Key discoveries

### Minimal LibFormat link closure (10 archives)

Derived from upstream `clang/lib/Format/CMakeLists.txt`. Linking all 41+ LLVM/clang
archives was unnecessary — `--gc-sections` produced the same module. Explicit list in
`link_libformat_wasm.sh` (two passes; wasm-ld has no `--start-group`):

`libclangFormat`, `libclangToolingInclusions`, `libclangToolingCore`, `libclangRewrite`,
`libclangLex`, `libclangBasic`, `libLLVMFrontendOpenMP`, `libLLVMTargetParser`,
`libLLVMSupport`, `libLLVMDemangle`.

### Hot path vs cold path

`reformat()` uses in-memory VFS via `SourceManagerForFile`. Real filesystem code was
linked from cold paths: `Format.cpp` config discovery, `FileManager` default ctor,
LLVMSupport `RealFileSystem` / `raw_fd_ostream` / `Path` / `Process`.

Patch `dprint-wasm-no-real-fs.patch` guards those on `__wasm__`. After rebuild,
`getRealFileSystem` is gone from final wasm; smoke still passes.

### Stub floor (~7 shipped)

Most POSIX stubs in source are link-time only; `--gc-sections` drops unused ones. After
patches, **7** stubs ship in the binary (`fstat`, `gmtime`, `localtime`, `stat`, `time`,
`unlink`, `write`) — mostly libc++ chrono + residual fs. Dropping `libcxx_chrono.o`
breaks link. `-DLLVM_ON_WIN32` experiment showed no meaningful reduction vs unix.

## Maintainability changes (2026-06-16/17)

- Removed probes, analysis scripts, bloated import reports.
- Split `wasm_support.cpp` → runtime core + POSIX stubs.
- Inlined `build_dprint_plugin.sh` into `link_libformat_wasm.sh`.
- Dropped unused win32 configure branch; updated `AGENTS.md`.

## Next axes (maintainability-first)

1. Keep POSIX stubs in one file; only add when link fails.
2. Prefer small upstream LLVM wasm guards over repo stub sprawl.
3. Defer libFormat subset extraction unless it reduces *source* complexity, not MiB.
4. Resist re-expanding link script to “link everything”.
