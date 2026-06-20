# AGENTS.md

This project builds a sandboxed dprint WebAssembly plugin for clang-format using
LLVM/Clang LibFormat.

## Project map

- `src/` contains the dprint Wasm plugin exports, LibFormat wrapper, fixed-heap
  allocator integration, and freestanding runtime shims.
- `cmake/` and `CMakeLists.txt` configure the `wasm32-unknown-unknown` plugin
  link against the LLVM/Clang LibFormat archive closure.
- `scripts/` contains LLVM fetch/configure helpers and the integration test
  driver.
- `tests/` contains the owned ABI and dprint CLI integration tests.
- `support/patches/` contains small LLVM patches for the freestanding Wasm
  build.
- `support/wasm-sysroot/` and `support/libcxx-wasm/` contain the minimal
  freestanding headers needed by the plugin build.

## Dev tool commands

- `mise install` installs the pinned host tools and hk hook.
- `mise run check` runs the configured repo checks.
- `mise run fix` formats repo metadata files with dprint. It intentionally does
  not run clang-format; the project should self-host C/C++ formatting after
  release.
- `mise run fetch-llvm` downloads pinned LLVM source and applies patches.
- `mise run configure-llvm-wasm` configures the LLVM/Clang Wasm cross-build.
- `mise run build-llvm-format` builds the LibFormat archive closure.
- `mise run configure-dprint-plugin` configures the plugin CMake build.
- `mise run test` builds and tests `build/plugin.wasm`.

## Project invariants

One maintainable zero-import Wasm dprint plugin: minimal repo source, explicit
LibFormat link closure, and as little support/shim code as possible. Wasm byte
size is secondary.

Strategies (maintainability-first):

- Build only the clang/LibFormat slice we need (explicit link closure, LLVM
  CMake trims).
- Patch LLVM in a small, reviewable way (`support/patches/`) to drop cold-path
  deps instead of growing POSIX stubs.
- Eliminate repo shims when upstream wasm guards or link GC make them
  unnecessary.

## Relevant Docs

- dprint: https://dprint.dev
- dprint plugins: https://dprint.dev/plugins/
- dprint Wasm plugin development:
  https://github.com/dprint/dprint/blob/main/docs/wasm-plugin-development.md
- dprint configuration: https://dprint.dev/config/
- Clang LibFormat: https://clang.llvm.org/docs/LibFormat.html
- LLVM getting started/building: https://llvm.org/docs/GettingStarted.html
- mise: https://mise.jdx.dev
- pixi: https://pixi.sh

## Notes

- Prefer validating the C++/Wasm build before introducing Rust.
- Do not use Emscripten output for the dprint plugin path.
- Do not add clang-format as a repo formatter before the plugin can self-host
  after release.
- Avoid filesystem-dependent `.clang-format` discovery in the sandboxed plugin;
  pass style configuration through dprint config instead.
- LLVM source lives in gitignored `third_party/`; wasm freestanding patches live
  in `support/patches/`.
- Success criterion: `mise run test` passes (zero Wasm imports, dprint CLI
  formats stdin).
