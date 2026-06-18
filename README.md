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

`fetch-llvm` downloads pinned LLVM 22.1.7 into `third_party/` and applies the
patches in `support/patches/` (via `scripts/apply_llvm_patches.sh`). To verify
patch reproducibility from a clean extract:

```sh
rm -rf third_party/llvm-project-22.1.7.src
pixi run fetch-llvm
```

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
- `support/patches/` — LLVM wasm freestanding guards (in-memory VFS, no real FS,
  signal/process/env trims)
- `support/wasm-sysroot/` and `support/libcxx-wasm/` — freestanding headers

## Gaps and future polish

Spike ABI is complete (`smoke-dprint-plugin` passes); production polish is not.

- [ ] **CI** — no automated fetch/build/smoke pipeline yet
- [ ] **Plugin metadata** — placeholder name, empty `helpUrl` / `configSchemaUrl`, POC license text
- [ ] **Config depth** — nested clang-format options (e.g. `IncludeCategories`, object arrays) are rejected; only flat PascalCase keys plus dprint globals
- [ ] **Style discovery** — reject or document unsupported `BasedOnStyle: file` / parent-config inheritance paths
- [ ] **`check_config_updates`** — always returns empty ok; no file-watch integration
- [ ] **Tests** — smoke covers full-file format only; no `format_range`, ObjC, invalid-range, or config-diagnostic cases
- [ ] **Wasm size / link audit** — optional import/symbol reports beyond the smoke import summary
- [ ] **Sysroot trim** — freestanding headers could be narrowed further if build stays stable

## Plugin surface

The plugin accepts C/C++/Obj-C-style extensions and formats the shared text
buffer with `clang::format::reformat`. Plugin config uses
[clang-format YAML option names](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
as JSON keys (PascalCase, e.g. `BasedOnStyle`, `ColumnLimit`). The legacy
`style` key is accepted as an alias for `BasedOnStyle`.

dprint global options map into clang-format when not overridden in
`clangFormat`:

| dprint global | clang-format |
|---------------|--------------|
| `lineWidth` | `ColumnLimit` |
| `indentWidth` | `IndentWidth` |
| `useTabs` | `UseTab` |

The plugin does not discover `.clang-format` files from the filesystem.

## Notes

- Final wasm has zero imports (no WASI, Emscripten, or host syscalls).
- LLVM source lives in gitignored `third_party/`; patches live in this repo.
