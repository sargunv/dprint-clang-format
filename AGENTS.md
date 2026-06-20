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
- `mise deps llvm` downloads pinned LLVM source and applies patches.
- `mise run configure` configures the LLVM/Clang Wasm build and plugin build.
- `mise run build` builds the LibFormat archive closure and plugin Wasm.
- `mise run test` builds, then tests `build/plugin.wasm`.

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
