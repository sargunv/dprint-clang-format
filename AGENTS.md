# AGENTS.md

This project is a spike toward a sandboxed dprint WebAssembly plugin for
clang-format using LLVM/Clang LibFormat.

## Goal

Validate whether LLVM/Clang LibFormat can be built into a non-Emscripten Wasm
module whose imports are compatible with dprint's Wasm plugin host. If viable,
wrap that formatter behind dprint schema v4 plugin exports.

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
- The first milestone is a small `probe.wasm` plus an import report.
