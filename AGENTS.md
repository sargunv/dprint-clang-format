# AGENTS.md

This project builds a sandboxed dprint WebAssembly plugin for clang-format using
LLVM/Clang LibFormat.

## Goal

One maintainable zero-import Wasm dprint plugin: minimal repo source, explicit LibFormat
link closure, and as little support/shim code as possible. Wasm byte size is secondary.

Strategies (maintainability-first):

- Build only the clang/LibFormat slice we need (explicit link closure, LLVM CMake trims).
- Patch LLVM in a small, reviewable way (`support/patches/`) to drop cold-path deps
  instead of growing POSIX stubs.
- Eliminate repo shims when upstream wasm guards or link GC make them unnecessary.

## Relevant Docs

- dprint: https://dprint.dev
- dprint plugins: https://dprint.dev/plugins/
- dprint Wasm plugin development: https://github.com/dprint/dprint/blob/main/docs/wasm-plugin-development.md
- dprint configuration: https://dprint.dev/config/
- Clang LibFormat: https://clang.llvm.org/docs/LibFormat.html
- LLVM getting started/building: https://llvm.org/docs/GettingStarted.html
- mise: https://mise.jdx.dev
- pixi: https://pixi.sh

## Notes

- Prefer validating the C++/Wasm build before introducing Rust.
- Do not use Emscripten output for the dprint plugin path.
- Avoid filesystem-dependent `.clang-format` discovery in the sandboxed plugin;
  pass style configuration through dprint config instead.
- LLVM source lives in gitignored `third_party/`; wasm freestanding patches live in
  `support/patches/`.
- Success criterion: `pixi run smoke-dprint-plugin` passes (zero Wasm imports, dprint CLI
  formats stdin).
