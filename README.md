# dprint clang-format Wasm spike

This repository builds a sandboxed dprint Wasm plugin that formats C/C++-style
source with LLVM/Clang LibFormat. The plugin is a dprint Wasm plugin, not a
process plugin, and it does not shell out to `clang-format`.

## Goal

Keep the repo small and maintainable: one Wasm plugin artifact, a minimal LibFormat
link closure, LLVM patches for freestanding wasm, and a single smoke test.

## Reproduce

```sh
mise install
mise run pixi-install
pixi run fetch-llvm
pixi run configure-llvm-wasm
pixi run ninja -C build/llvm-wasm clangFormat
pixi run smoke-dprint-plugin
```

`fetch-llvm` downloads pinned LLVM 22.1.7 into `third_party/` and applies
`support/patches/dprint-wasm-no-real-fs.patch`.

`smoke-dprint-plugin` builds `build/dprint-clang-format-plugin.wasm`, writes a
short import summary to `reports/dprint-plugin-imports.md`, exercises the plugin
ABI from Node, and runs the dprint CLI:

```sh
printf 'int main(){return 1;}\n' |
  dprint fmt --stdin cpp --config-discovery=false \
    --plugins build/dprint-clang-format-plugin.wasm
```

Expected formatted output:

```cpp
int main() { return 1; }
```

## Layout

- `src/dprint_plugin.cpp` — dprint schema v4 exports and LibFormat wrapper
- `src/wasm_runtime_core.cpp` — freestanding allocator, mem*, and C++ runtime shims
- `scripts/link_libformat_wasm.sh` — links the plugin against a minimal LibFormat
  archive set (10 LLVM/clang static libraries, two passes)
- `scripts/configure_llvm_wasm.sh` — cross-build LLVM/Clang for
  `wasm32-unknown-unknown`
- `support/patches/dprint-wasm-no-real-fs.patch` — in-memory VFS and wasm I/O trims
- `support/wasm-sysroot/` and `support/libcxx-wasm/` — freestanding headers
- `worklog.md` — simplification notes and discoveries

## Plugin surface

The plugin accepts C/C++/Obj-C-style extensions and formats the shared text
buffer with `clang::format::reformat`. Inline config supports `style`:
`LLVM`, `Google`, `WebKit`, `Microsoft`.

The plugin does not discover `.clang-format` files from the filesystem.

## Notes

- Final wasm has zero imports (no WASI, Emscripten, or host syscalls).
- LLVM source lives in gitignored `third_party/`; patches live in this repo.
- See `worklog.md` for the active simplification plan.
