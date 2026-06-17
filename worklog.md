# Worklog: simplify libformat Wasm dprint plugin

Goal: keep the repo maintainable with one zero-import Wasm dprint plugin backed by a
minimal LibFormat link closure. Wasm byte size is secondary; fewer scripts, probes, and
support files matter more.

## 2026-06-16 — maintainability pivot

- Removed freestanding/libformat/stl probes and their build/report/smoke tasks.
- Removed loop-era analysis scripts (`analyze_*`, `report_shipped_stubs`).
- Slimmed `pixi.toml` to fetch → configure → build → smoke.
- `report_wasm_imports.sh` now writes a short import summary (no full objdump dump).
- Split runtime: `wasm_runtime_core.cpp` + `wasm_posix_stubs.cpp`.
- LLVM patch `support/patches/dprint-wasm-no-real-fs.patch` applied by `fetch-llvm`.

Next simplification axes (maintainability-first):

1. Document the ten-archive link set inline in `link_libformat_wasm.sh` (done) and
   resist re-expanding to “link everything”.
2. Keep POSIX stubs in one small file; only add stubs when link fails.
3. Prefer upstream LLVM wasm guards over repo-local stub sprawl when patches stay small.
4. Defer libFormat subset extraction unless it reduces *source* complexity, not just MiB.

## 2026-06-16 — loop iteration: docs and configure trim

- Updated `AGENTS.md` to match the single-plugin, maintainability-first goal.
- Dropped unused `LLVM_WASM_HOST=win32` branch from `configure_llvm_wasm.sh` (experiment
  showed no meaningful stub reduction vs unix).
- Inlined `build_dprint_plugin.sh` defaults into `link_libformat_wasm.sh` (one fewer script).

## 2026-06-16 — baseline and dependency audit

### Current state (verified)

- `pixi run smoke-dprint-plugin` passes: formats stdin via dprint CLI, zero Wasm imports.
- Final plugin size: ~3.0–3.2 MiB (`build/dprint-clang-format-plugin.wasm`).
- LLVM 22.1.7 cross-built for `wasm32-unknown-unknown`; LibFormat linked in-process.

### Discovery: link script was over-linking, but `--gc-sections` hid it

`scripts/link_libformat_wasm.sh` previously linked **every** `libclang*.a` and `libLLVM*.a`
under `build/llvm-wasm/lib` (41 archives), repeated four times for circular deps.

Static archive totals are ~60 MiB (e.g. `libLLVMAnalysis.a` 8.9M, `libLLVMCore.a` 7.2M,
`libLLVMObjectYAML.a` 4.4M). None of those analysis/scalar-opt archives are required for
LibFormat.

**Minimal link set** (derived from upstream `clang/lib/Format/CMakeLists.txt` and
transitive `LINK_LIBS`):

| Archive | Role |
|---------|------|
| `libclangFormat.a` | formatter |
| `libclangToolingInclusions.a` | include-style helpers |
| `libclangToolingCore.a` | `Replacement` / diagnostics |
| `libclangRewrite.a` | apply replacements |
| `libclangLex.a` | lexer / preprocessor |
| `libclangBasic.a` | tokens, diagnostics, file manager |
| `libLLVMSupport.a` | strings, ADT, regex, process stubs |
| `libLLVMTargetParser.a` | target triple parsing (clangBasic/clangLex dep) |
| `libLLVMFrontendOpenMP.a` | clangBasic dep |
| `libLLVMDemangle.a` | demangle (Support dep) |

Tested: linking only these ten archives produces the **same ~3.2 MiB module** with
`--gc-sections`. Extra archives were dead weight at link time, not in the final binary.

**Action taken:** updated `link_libformat_wasm.sh` to use this explicit list (two passes
for wasm-ld ordering; `--start-group` is unsupported). Smoke test still passes.

Added `scripts/analyze_runtime_symbols.sh` (`pixi run analyze-runtime-symbols`) to
re-run the static archive vs `wasm_support.o` cross-check when trimming stubs.

### Discovery: most `wasm_support.cpp` stubs are genuinely referenced

Static analysis (`llvm-nm -u` on LLVM/clang archives vs defined symbols in
`wasm_support.o`):

- ~144 of ~189 C/POSIX/runtime symbols in `wasm_support.cpp` are referenced from
  LLVM/clang object code (via `LLVMSupport` paths compiled with `-DLLVM_ON_UNIX`).
- ~54 symbols appear unreferenced in static archive scan (mostly wide-char helpers,
  some `operator delete`/`new` alignment variants, `gettimeofday`, `waitpid`, etc.).
- With `-ffunction-sections` + `--gc-sections`, unreferenced stub **functions** are
  already dropped from the final Wasm; the source file is large but not all of it lands
  in the binary.

**Implication:** trimming `wasm_support.cpp` is mostly a **maintainability** win unless
we also stop LLVM from compiling Unix I/O paths.

### Discovery: libFormat itself is small; the closure is not

- `libclangFormat.a` is ~1.5 MiB of source, 23 `.cpp` files.
- Real weight comes from **clangLex** (full preprocessor/HeaderSearch) and
  **LLVMSupport** (filesystem, process, regex, command-line infra).
- `Format.cpp` contains `.clang-format` file discovery (`llvm::vfs::getRealFileSystem`)
  and `loadAndParseConfigFile`. Our dprint plugin calls `reformat()` with an in-memory
  `FormatStyle` and never hits those paths, but the code is still linked today.

### Simplification axes (ordered by impact)

1. **Link hygiene (done this session):** explicit minimal archive list; faster links,
   clearer dependency documentation.
2. **LLVM build flags (next):** try dropping `-DLLVM_ON_UNIX` or enabling
   `-DLLVM_ON_WIN32` in `configure_llvm_wasm.sh` to reduce Support's Unix surface;
   measure which `wasm_support` stubs become unnecessary.
3. **Runtime stub split (next):** separate `wasm_runtime_core.cpp` (allocator, mem*,
   C++ operators, compiler helpers) from `wasm_posix_stubs.cpp` so POSIX baggage is
   obviously optional/deletable.
4. **LibFormat subset extraction (larger):** upstream libFormat pulls clangLex +
   LLVMSupport. A true "pure formatter" would need either:
   - an LLVM-side `clangFormatMinimal` target that omits filesystem config loading and
     trims Lex to what tokenization requires, or
   - vendoring Format sources with stubbed dependencies (high maintenance).
5. **Report noise (minor):** import reports embed full `wasm-objdump -x` (~79k lines);
   README already lists summarizing these as follow-up.

### Open questions

- Can `configure_llvm_wasm.sh` request a narrower Support build (fewer subsystems) via
  existing LLVM CMake knobs without patching?
- Does `-DLLVM_ON_UNIX=0` still compile and pass smoke tests?
- Which clangLex units does `FormatTokenLexer` actually need vs full preprocessor?

## 2026-06-17 — stub trim pass (immediate loop)

Removed ~150 lines of dead runtime stubs from `wasm_support.cpp` after
`analyze-runtime-symbols` cross-check:

- entire wide-char stub block (`wcs*`, `wcst*`, `swprintf`, `wmemcmp`, …)
- unused POSIX/misc: `strcpy`, `strsignal`, `strtok_r`, `strtold`, `labs`/`llabs`/`div`/`ldiv`/`lldiv`
- unused FILE helpers: `feof`, `ferror`, `fflush`
- unused process/time: `gettimeofday`, `signal`, `waitpid`, `rmdir`, `setsockopt`, `getpwuid`

`pixi run smoke-dprint-plugin` still passes; wasm size unchanged (~3.0 MiB).

Next: try `-DLLVM_ON_WIN32` instead of
`-DLLVM_ON_UNIX` in `configure_llvm_wasm.sh` to shrink LLVM's reachable I/O surface.

## 2026-06-17 — split runtime support

Split monolithic `wasm_support.cpp` into:

- `src/wasm_runtime_core.cpp` (~440 lines): allocator, mem/string helpers, ctype,
  str conversion, compiler runtime (`__multi3`, `__cxa_*`), C++ operators
- `src/wasm_posix_stubs.cpp` (~465 lines): POSIX/process/fs/network/dl stubs required
  by `LLVMSupport` today

Updated link/build/analyze scripts accordingly; deleted `wasm_support.cpp`.
Smoke test still passes.

## 2026-06-17 — LLVM host-style experiment

Added `LLVM_WASM_HOST=unix|win32` to `configure_llvm_wasm.sh`. Kicked off a parallel
`-DLLVM_ON_WIN32` build at `build/llvm-wasm-win32` to see if Support pulls fewer POSIX
symbols than the current `-DLLVM_ON_UNIX` path.

First attempt failed: fresh build dir had no `NATIVE/bin/llvm-tblgen`. Fixed configure
to reuse host tablegen tools from `build/llvm-wasm/NATIVE/bin` when the target build
dir lacks them.

Retry succeeded: `ninja -C build/llvm-wasm-win32 clangFormat` in ~72s. Plugin smoke
test passes when linked against win32 archives (`LLVM_WASM_BUILD=build/llvm-wasm-win32`).

Symbol diff on the 10-archive closure: 1478 undefined (unix) vs 1476 (win32) — essentially
no win. Win32 adds `__imp_*` stdcall imports; unix adds `fork`/`wait`/`uname` etc. Neither
path meaningfully reduces `wasm_posix_stubs.cpp` surface. **Conclusion:** host-style flag
alone is not the lever; need LLVM Support feature trimming or libFormat subset extraction.

Win32 closure still references 142/162 runtime symbols (unix ~144) — no practical stub
reduction. Next targets in LLVMSupport: `Path.cpp`, `Process.cpp`, `Program.cpp`,
`VirtualFileSystem.cpp`, `CommandLine.cpp` (filesystem/process even though plugin only
uses in-memory `reformat()`).

Per-object `llvm-nm` on extracted Support members confirms these compile against
`llvm::sys::fs::*` and `Process::*` abstractions, but those abstractions still resolve
to our POSIX stubs at link time via other Support translation units. `Path.cpp.obj` alone
references ~40 C stubs; `Program.cpp.obj` adds `fork`/`execve`/`wait`.

Final wasm still retains `llvm::vfs::getRealFileSystem()` and `RealFileSystem::getRealPath`
(symbols present in linked module), so `--gc-sections` is not dropping filesystem code —
LibFormat/clangBasic paths keep it reachable despite the plugin using in-memory `reformat()`.

**Important nuance:** the hot path *does* use in-memory VFS. `Environment::make` →
`SourceManagerForFile` constructs `FileManager` with `InMemoryFileSystem`, not
`getRealFileSystem()` (see `SourceManager.cpp:2324-2335`). The retained real-filesystem
code is from other translation units: `Format.cpp` config-file discovery (~4435),
`FileManager` default ctor fallback, and LLVMSupport `RealFileSystem` implementation.
Patching or `#ifdef`-ing those cold paths (not replacing `reformat()` itself) is the
promising trim vector.

## 2026-06-17 — wasm VFS patch (cold-path trim)

Applied `support/patches/dprint-wasm-no-real-fs.patch` via `scripts/apply_llvm_patches.sh`:

- `FileManager` default ctor uses `InMemoryFileSystem` on `__wasm__` instead of
  `getRealFileSystem()`.
- `getStyle()` in `Format.cpp` errors instead of opening the real filesystem on wasm.

After rebuild: `getRealFileSystem` / `RealFileSystem::getRealPath` **gone** from final
wasm; smoke test still passes; size still ~3.0 MiB. Static archive analysis still lists
144/162 stubs (Support objects still *compile* against POSIX), but dead real-FS code is
no longer linked into the plugin module.

Post-patch check on final wasm: only **16** POSIX stub functions actually land in the
binary (e.g. `open`, `stat`, `close`, `write`, `sysconf`, `time`) — `fork`/`execve`/
`mmap`/`realpath`/`dlopen` are GC'd out. Stubs for GC'd symbols are still required at
**link** time because static archives reference them, but they no longer bloat the shipped
module.

Wired patch application into `fetch_llvm.sh` / `pixi run apply-llvm-patches` so fresh
LLVM checkouts get the wasm VFS changes automatically.

Added `pixi run report-shipped-stubs` to track link-only vs shipped POSIX stubs (currently
16 shipped / 70 link-only).

Remaining shipped stubs (`open`, `close`, `write`, `lseek`, `stat`, …) correlate with
`llvm::raw_fd_ostream` still being reachable; `Path.cpp` / `Program.cpp` symbols are no
longer in the final wasm. Next trim: stub or `#if __wasm__` guard `raw_fd_ostream` file
open paths in LLVMSupport.

Applied wasm guards in `raw_ostream.cpp` (`getFD` + fd ctor skip `lseek`/`stat`). Shipped
POSIX stubs: 16 → **15** (`open` dropped; smoke test still passes).

Further `raw_ostream` wasm no-ops in `write_impl`/`seek`/`pwrite_impl`: **15 → 14**
(`lseek` dropped; `write` still shipped).

`Process.inc` wasm stubs for `FixupStandardFileDescriptors` / `SafelyCloseFileDescriptor` /
`FileDescriptorIsDisplayed`: **14 → 10** shipped stubs.

`Path.inc` `home_directory` wasm early-return: **10 → 7** (`getpwuid_r`, `getuid`, `sysconf`
dropped). Remaining 7 are mostly libc++ chrono (`gmtime`, `localtime`, `time`) plus
`write`/`stat`/`fstat`/`unlink`. Dropping `libcxx_chrono.o` fails link (required by
`FileManager` / `VirtualFileSystem` time APIs) — likely near-term floor without libc++
/ VFS time patches.

Regenerated `support/patches/dprint-wasm-no-real-fs.patch` from pristine 22.1.7 tarball
(`patch --dry-run` clean on fresh extract).

**Loop plateau:** further 1s ticks unlikely to shrink shipped stubs without new LLVM/libc++
patches (chrono/time floor at 7). Next high-value work: VFS time-stamp stubs or libFormat
subset extraction.

## 2026-06-17 — loop reset: maintainability framing (user)

User clarified priorities:

- **Maintainability > wasm MiB.** Reduce repo *source* size (support/shims, scripts).
- **`worklog.md` is the progress monitor** — append discoveries; do not condense it.
- Target: one simple plugin with just enough clang/LibFormat; strategies include building
  less LLVM, maintainable patches, eliminating shims.

Reverted mistaken worklog condense (`bf6daec`). Updated `AGENTS.md` and loop prompt.

### Link-time stub audit

Cross-checked `llvm-nm -u` on the 10-archive closure vs symbols defined in
`wasm_posix_stubs.o` / `wasm_runtime_core.o`:

- 2135 link-time undefined symbols total (mostly C++ runtime).
- 80/83 posix stub functions are referenced at link; only **3 unused**:
  `getuid`, `isatty`, `getpwuid_r` (leftover after `Path.inc` `home_directory` wasm patch).

Removed those three from `wasm_posix_stubs.cpp`. `pixi run smoke-dprint-plugin` still passes;
posix stub count 83 → **80** (~20 lines removed).

### Runtime core link-time trim

Same `llvm-nm -u` cross-check on `wasm_runtime_core.o`: **13** of 70 defined symbols had
no link-time references (unused aligned/nothrow `operator new/delete` overloads, `atoi`,
`std::__libcpp_verbose_abort`). Removed them; kept the overloads the closure actually
pulls in (`operator new(size_t, nothrow)`, `operator new(size_t, align, nothrow)`, sized
deletes). Smoke still passes; ~55 lines removed from `wasm_runtime_core.cpp`.

Follow-up audit: **0 unused posix stubs** remain. Removed one more unreferenced
`operator delete[](void*, size_t, align_val_t)`; runtime core link-time floor reached
(57/57 defined symbols referenced). Further support-code shrink needs LLVM patches
(e.g. VFS time/chrono) or building less of clang, not more stub pruning.

### Drop libLLVMFrontendOpenMP from link closure

`clang/lib/Basic/CMakeLists.txt` lists `FrontendOpenMP` in `LLVM_LINK_COMPONENTS`, but
`llvm-nm` + trial link show **no undefined symbols** from that archive in our formatter
path. Removed `libLLVMFrontendOpenMP.a` from `link_libformat_wasm.sh` (10 → **9**
archives). `pixi run smoke-dprint-plugin` still passes; wasm still ~3.0 MiB.

Note: LLVM still *builds* the archive when compiling clangBasic; skipping that would need
a wasm-specific CMake patch. Link closure trim is zero-risk for now.

### Link closure audit (8 required archives, 8 libcxx objects)

Trial link omitting one archive/object at a time:

| Removed | Result |
|---------|--------|
| `libLLVMFrontendOpenMP` | OK (already dropped) |
| `libLLVMTargetParser` | FAIL — `llvm::Triple` undefined from clangLex |
| `libLLVMDemangle` | FAIL — demangle symbols from Support |
| `libclangToolingInclusions` | FAIL |
| `libclangToolingCore` | FAIL — `applyAllReplacements` |
| `libclangRewrite` | FAIL — `Rewriter::ReplaceText` |
| each `libcxx_*.o` | FAIL — all eight objects required |

**Conclusion:** the nine-archive / eight-libcxx-object closure is minimal for link. Further
repo shrink is upstream-side (patch cold paths, skip building unused LLVM libs like
FrontendOpenMP in the wasm cross-build).

### Patch: skip building libLLVMFrontendOpenMP on wasm

Added `support/patches/dprint-wasm-skip-frontend-openmp.patch`: on
`CMAKE_CXX_COMPILER_TARGET` matching `wasm32`, drop `FrontendOpenMP` from
`clang/lib/Basic/CMakeLists.txt` `LLVM_LINK_COMPONENTS`. Refactored
`apply_llvm_patches.sh` to apply multiple patches idempotently.

After reconfigure + rebuild: `libLLVMFrontendOpenMP.a` is **no longer produced** in
`build/llvm-wasm/lib/` (deleted artifact; `ninja clangFormat` does not recreate it).
Smoke still passes.

### Patch: fixed __DATE__/__TIME__/__TIMESTAMP__ on wasm

Added `support/patches/dprint-wasm-fixed-date-time.patch` on
`clang/lib/Lex/PPMacroExpansion.cpp`: `ComputeDATE_TIME` and `__TIMESTAMP__` expand to
fixed epoch strings without calling `time`/`gmtime`/`localtime`. Removes those refs from
`libclangLex.a` (verified via `llvm-nm -u`).

Shipped POSIX stubs in final wasm: **7 → 4** (`fstat`, `stat`, `unlink`, `write`). Removed
now-unreferenced `time`/`gmtime`/`localtime` wrappers from `wasm_posix_stubs.cpp`; kept
`gmtime_r`/`localtime_r`/`strftime` (still required at link by libc++ chrono path).
Smoke passes.

### Patch: trim LLVMSupport fs deps on wasm

Added `support/patches/dprint-wasm-trim-fs-deps.patch` — wasm guards on cold-path
`stat`/`fstat`/`write`/`unlink` users in LLVMSupport (`Path.inc`, `Path.cpp`,
`raw_ostream::preferred_buffer_size`, `ErrorHandling`, `Signals`, `Jobserver`,
`raw_socket_stream`). Link-time refs to `stat`/`fstat`/`write`/`unlink` in our
closure drop to **zero**; removed the four matching stubs from `wasm_posix_stubs.cpp`
(~30 lines). **Zero POSIX stubs ship** in final wasm (`llvm-nm` check). Smoke passes.

### POSIX stub plateau (link-only shims)

Post fs-trim audit: **73** functions in `wasm_posix_stubs.cpp`, all **73** referenced at
link time, **0** unused, **0** shipped in final wasm (`--gc-sections` drops the entire
stub object’s C functions from the binary — they exist only to satisfy static archive
undefined symbols). Further shrink needs more LLVM wasm guards (e.g. `fork`/`exec`/
`socket`/`mmap` in LLVMSupport cold paths), not more stub pruning.

### Patch: disable Unix process spawn/wait on wasm

Extended LLVM patching: `Program.inc` `Execute`/`Wait` bodies wrapped in
`#ifndef __wasm__` with early error returns. Drops link-time refs to `fork`,
`execv`, `execve`, `wait`, `wait4`, `kill`, `setsid`, `_exit` — removed **8**
matching stubs from `wasm_posix_stubs.cpp` (**73 → 65** link-only shims). Smoke
passes.

### Patch: trim mmap/dl/socket support on wasm

Added `support/patches/dprint-wasm-trim-support-deps.patch` — wasm guards on
`DynamicLibrary`, `Memory`, `Path` mapped files, and `raw_socket_stream`. Drops
link-time refs to `dlopen`/`dlsym`/`dlclose`/`dlerror`/`dladdr`, `mmap`/`munmap`/
`msync`, and socket/`pipe` APIs. Removed **14** matching stubs + unused headers
(**65 → 51** link-only shims). Smoke passes.

Follow-up link audit: **1** unused stub remained (`dup2`, leftover from `Program.inc`
wasm guard); removed it (**51 → 50**). All remaining posix shims are link-required.

### Patch: jobserver/RNG/socket poll wasm guards

Extended `dprint-wasm-trim-support-deps.patch`: disable `JobserverClientImpl` I/O on
wasm, use zero-fill instead of `/dev/urandom` in `getRandomBytes`, and skip
`manageTimeout`/`poll`. Removed **2** more unused stubs (`dup`, `poll`); **50 → 48**
link-only shims. `open`/`close`/`read` still required via `Path.cpp`/`raw_ostream` —
next trim target.

### Path.cpp/Path.inc: use `#else` wasm guards (not early `#endif`)

Prior wasm guards returned early but left POSIX calls in the compiled object
(`#return` then `#endif` still emits `read`/`close`/etc. at link). Reworked
Path.cpp and Path.inc cold paths to wrap bodies in `#else`/`#endif`, including
`copy_file*`, `md5_contents`, temp-file closes, directory iteration,
`current_path`/`set_current_path`/`access`/`real_path`, `readNativeFile*`, and
`getMainExecutable`. Removed **8** now-unused stubs (`read`, `close`, `access`,
`chdir`, `getcwd`, `lseek`, `readdir`, `closedir`); **45 → 37** link-only
shims. Smoke passes.

### Path.inc patch backfill

Added `dprint-wasm-trim-path-posix-inc.patch` (plus `apply_llvm_patches.sh` hook) so
Path.inc wasm guards survive fresh `fetch-llvm`. Also fixed `status()`/`status(FD)`
to use `#else` (same early-`#endif` bug).

### CrashRecoveryContext/Signals wasm guards

Disabled POSIX crash recovery (`setjmp`/`longjmp`, signal install) and
`RegisterHandlers`/`unregisterHandlers` on wasm. Removed **7** unused signal
stubs (`sigaction`, `longjmp`, `raise`, `sigfillset`, `sigemptyset`, `sigaddset`,
`sigprocmask`); **37 → 30** link-only shims. Smoke passes.

### getenv/path env wasm guards

Guarded `getenv` cold paths: `Process::GetEnv`, terminal columns/colors,
`findProgramByName`, Jobserver `MAKEFLAGS`, stack symbolizer helpers, plus
Path.inc `remove`, tilde expansion, `home_directory`, XDG/temp env, and
`madvise` helpers. Removed **4** unused stubs (`remove`, `lstat`, `madvise`,
`getpwnam_r`); **30 → 26** link-only shims. Smoke passes.
