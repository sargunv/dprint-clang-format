# dprint clang-format Wasm spike

This repository is a proof of concept for running LLVM/Clang LibFormat inside
dprint's sandboxed Wasm plugin host. The plugin is a dprint Wasm plugin, not a
process plugin, and it does not shell out to `clang-format`.

## Result

The core architecture is viable.

LLVM/Clang LibFormat can be cross-built into a non-Emscripten
`wasm32-unknown-unknown` module and wrapped behind dprint schema v4 exports. The
current plugin formats C/C++-style source by calling `clang::format::reformat`
in-process, loads in the dprint CLI, and has no Wasm imports.

Verified artifacts:

- `build/probe.wasm`: tiny freestanding C++ sanity probe.
- `build/libformat-probe.wasm`: direct LibFormat in-memory formatting probe.
- `build/dprint-clang-format-plugin.wasm`: dprint schema v4 plugin wrapper.
- `reports/probe-imports.md`: freestanding probe import report.
- `reports/libformat-probe-imports.md`: LibFormat probe import report.
- `reports/dprint-plugin-imports.md`: dprint plugin import report.

All generated import reports currently show `Import count: 0`, so the proof of
concept has no WASI, Emscripten, syscall, or dprint host imports.

## Reproduce

Install the pinned tools, fetch LLVM, build Clang's LibFormat archive for Wasm,
then run the plugin smoke test:

```sh
mise install
mise run pixi-install
pixi run fetch-llvm
pixi run smoke-freestanding
pixi run configure-llvm-wasm
pixi run ninja -C build/llvm-wasm clangFormat
pixi run smoke-dprint-plugin
```

`smoke-dprint-plugin` builds `build/dprint-clang-format-plugin.wasm`, regenerates
`reports/dprint-plugin-imports.md`, instantiates the wasm directly from Node to
exercise the shared-byte ABI, then runs the real dprint CLI against the local
plugin:

```sh
printf 'int main(){return 1;}\n' |
  dprint fmt --stdin cpp --config-discovery=false \
    --plugins build/dprint-clang-format-plugin.wasm
```

Expected formatted output:

```cpp
int main() { return 1; }
```

The smoke test also writes a temporary dprint config with
`clangFormat.style = "Microsoft"` and verifies that inline plugin config reaches
the Wasm plugin.

## How It Works

Tooling is pinned in `mise.toml` and `pixi.toml`. Pixi supplies Clang/LLD,
CMake, Ninja, Python, and WABT. Mise pins `dprint` for the CLI smoke test.

LLVM source is pinned to `22.1.7` and fetched by `scripts/fetch_llvm.sh` into
`third_party/`, which is intentionally ignored by git.

The Wasm build avoids Emscripten and WASI:

- `scripts/configure_llvm_wasm.sh` configures LLVM/Clang for
  `wasm32-unknown-unknown`.
- `support/wasm-sysroot/include/` provides the minimal C/POSIX headers needed to
  compile the reachable LLVM/Clang code.
- `support/libcxx-wasm/include/` overlays libc++ headers to disable or stub
  unsupported runtime facilities.
- `src/wasm_support.cpp` provides freestanding runtime functions, allocator
  support, libc/POSIX stubs, C++ allocation hooks, and compiler helper symbols.
- `scripts/link_libformat_wasm.sh` links the entry object, support runtime,
  selected libc++ objects, and LLVM/Clang static archives into a standalone wasm
  module.

The dprint plugin wrapper lives in `src/dprint_plugin.cpp`. It exposes schema v4
exports such as `dprint_plugin_version_4`, `get_plugin_info`,
`register_config`, `format`, and `get_formatted_text`. Text and JSON payloads
move through dprint's shared-byte ABI.

## Current Plugin Surface

The plugin accepts C/C++/Obj-C-style file extensions and formats the current
shared text buffer with LibFormat. It supports a minimal inline `style` setting:

- `LLVM`
- `Google`
- `WebKit`
- `Microsoft`

The plugin intentionally does not discover `.clang-format` files from the
filesystem. Sandboxed dprint plugins should receive configuration through dprint
config instead.

## What This Proves

This spike proves the high-risk integration points:

- LibFormat can be cross-built for non-Emscripten Wasm.
- A standalone LibFormat wasm can format an in-memory string.
- The resulting modules can have zero imports.
- A dprint schema v4 wrapper can call LibFormat in-process.
- dprint 0.54.0 can load the local wasm plugin and format stdin.
- Inline dprint config can be passed into the plugin and affect formatting.

## What Remains

This is not production-ready. The main remaining work is productization and
correctness breadth:

- Replace substring-based config detection with real JSON/config parsing.
- Define a proper config schema and useful config diagnostics.
- Map more clang-format options onto `clang::format::FormatStyle`.
- Add broad formatting fixtures for C, C++, headers, Obj-C, and Obj-C++.
- Audit memory behavior on large files and real projects.
- Trim the LLVM/Clang archive link closure and reduce wasm size.
- Strip nonessential custom/name sections from release artifacts.
- Split the freestanding runtime into intentional support vs temporary stubs.
- Keep summarized import reports in git instead of the current large full
  `wasm-objdump -x` reports.
- Add CI for the reproducible smoke path.

The important conclusion is that the sandboxed dprint Wasm plugin approach is
not blocked by LibFormat's build/link/runtime requirements.
