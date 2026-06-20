# dprint clang-format Wasm plugin

This repository builds a sandboxed dprint Wasm plugin that formats C/C++-style
source with LLVM/Clang LibFormat. The plugin is a dprint Wasm plugin, not a
process plugin, and it does not shell out to `clang-format`.

## Goal

Keep the repo small and maintainable: one Wasm plugin artifact, a minimal
LibFormat link closure, LLVM patches for freestanding wasm, and focused
integration tests for the plugin behavior this repo owns.

## Reproduce

```sh
mise install
mise run configure
mise run build
mise run test
```

C/C++ formatting is intentionally not wired to clang-format; this repo should
self-host that after release.

The `llvm` mise dependency downloads pinned LLVM 22.1.7 into `third_party/` and
applies the patches in `support/patches/` (via `scripts/fetch_llvm.sh`). It runs
automatically before `mise run ...`. To verify patch reproducibility from a
clean extract:

```sh
rm -rf third_party/llvm-project-22.1.7.src
mise deps llvm
```

`mise run test` builds `build/plugin.wasm`, verifies the module has zero
imports, and exercises the plugin through dprint CLI commands:

```sh
printf 'int main(){return 1;}\n' |
  dprint fmt --stdin cpp --config-discovery=false \
    --plugins build/plugin.wasm
```

Expected formatted output:

```cpp
int main() { return 1; }
```

## Layout

- `src/dprint_plugin.cpp` — dprint schema v4 exports and LibFormat wrapper
- `src/wasm_allocator.c` — vendored dlmalloc integration for fixed-memory Wasm
- `src/wasm_runtime_core.cpp` — mem*, freestanding libc, and C++ runtime shims
- `tests/` — Bats integration tests that exercise the plugin through dprint
- `schema.json` — permissive dprint config schema for the clang-format option
  map
- `CMakeLists.txt` — builds and links the plugin against a minimal LibFormat
  archive set and drives the nested LLVM native-tool and Wasm LibFormat builds
- `support/patches/` — LLVM wasm freestanding guards (in-memory VFS, no real FS,
  signal/process/env trims)
- `support/wasm-sysroot/` and `support/libcxx-wasm/` — freestanding headers

## Release status

CI is deferred; use the reproduce/test commands above as the release gate.

Known release semantics:

- `.clang-format` filesystem discovery is not supported in the sandboxed plugin.
  `BasedOnStyle: file` and `InheritParentConfig` are reported as config
  diagnostics.
- `check_config_updates` returns an empty ok response because there are no
  config migrations yet.
- Wasm size/link reports and further sysroot trimming are optional future
  polish.

## Plugin surface

The plugin accepts C/C++/Obj-C-style extensions and formats the shared text
buffer with `clang::format::reformat`. Plugin config uses
[clang-format YAML option names](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
as JSON keys (PascalCase, e.g. `BasedOnStyle`, `ColumnLimit`).

dprint global options map into clang-format when not overridden in
`clangFormat`:

| dprint global | clang-format  |
| ------------- | ------------- |
| `lineWidth`   | `ColumnLimit` |
| `indentWidth` | `IndentWidth` |
| `useTabs`     | `UseTab`      |

The plugin does not discover `.clang-format` files from the filesystem.

## Notes

- Final wasm has zero imports (no WASI, Emscripten, or host syscalls).
- Allocation uses a vendored dlmalloc copy from wasi-libc with mmap/morecore
  disabled, initialized from the module's fixed Wasm heap.
- LLVM source lives in gitignored `third_party/`; patches live in this repo.
