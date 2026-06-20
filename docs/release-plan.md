# Release plan

This repository has proven the hard part: a dprint schema v4 Wasm plugin can link
Clang LibFormat, run without Wasm imports, and format stdin through the dprint
CLI. The remaining work is mostly turning the spike into a maintainable release
artifact.

The first public release should be framed as experimental, but it should not ship
with prototype metadata, an unbounded bump allocator, or unclear configuration
semantics.

## Target release shape

- One committed source tree that reproducibly builds `plugin.wasm`.
- One GitHub release asset named `plugin.wasm`.
- Optional GitHub release asset named `schema.json`.
- A plugin info object with real `name`, `version`, `configKey`, `helpUrl`,
  `configSchemaUrl`, and `updateUrl`.
- A small integration test suite covering the dprint ABI behavior we own.
- No tracked prototype reports or historical audit artifacts.

For `plugins.dprint.dev`, the GitHub release convention is:

- `plugin.wasm` for Wasm plugins.
- `schema.json` when a schema exists.
- tag names without dashes, preferably `x.x.x`.
- URLs such as `https://plugins.dprint.dev/<user>/<repo>-<tag>.wasm`,
  `https://plugins.dprint.dev/<user>/<repo>/<tag>/schema.json`, and
  `https://plugins.dprint.dev/<user>/<repo>/latest.json`.

## 1. Remove prototype artifacts

Delete tracked artifacts that only documented the spike:

- `reports/dprint-plugin-imports.md`
- `scripts/report_wasm_imports.sh`, unless folded into a test or release check
- any unused probe/audit scripts if they are added later

Keep scripts that are part of the reproducible build path:

- `scripts/fetch_llvm.sh`
- `scripts/apply_llvm_patches.sh`
- `scripts/configure_llvm_wasm.sh`
- `scripts/link_libformat_wasm.sh`
- `scripts/smoke_dprint_plugin.sh`, renamed or replaced by tests
- `scripts/check_tools.sh`, if still useful for debugging local setup

After cleanup, the README should stop describing the repo as a spike and should
document the supported release path:

```sh
pixi run fetch-llvm
pixi run configure-llvm-wasm
pixi run ninja -C build/llvm-wasm clangFormat
pixi run build-dprint-plugin
pixi run test
```

## 2. Replace the allocator

Current state: `src/wasm_runtime_core.cpp` uses a bump allocator, ignores
`free`, has an unsafe `realloc` copy size, and has no heap bound check. That is
acceptable for proving the link, but it is the highest-risk launch blocker for a
long-lived dprint process.

Recommended path:

1. Integrate an existing allocator with a C-compatible
   `malloc`/`calloc`/`realloc`/`free` surface first. Do not implement allocator
   internals ourselves.
2. Keep the plugin zero-import and freestanding.
3. Add an allocator stress integration test that formats repeatedly in one Wasm
   instance.

Allocator candidates:

- `dlmalloc`: best first candidate. Rust's `wasm32-unknown-unknown` target uses
  a dlmalloc implementation as its default global allocator, and dlmalloc is a
  known fit for syscall-free Wasm memory management. Prefer vendoring or linking
  an existing implementation with a thin environment wrapper over writing
  allocator logic ourselves.
- Emscripten's `emmalloc`: worth investigating as a small embeddable allocator,
  provided it can be used without Emscripten runtime assumptions.
- Rust wrapper: attractive if we decide to use `dprint-core`'s generated Wasm
  ABI wrapper, but it adds a Rust/C++ FFI boundary around LibFormat. Do this only
  if it removes more support code than it adds.
- Zig allocator shim: possible, because Zig makes allocator export patterns
  straightforward, but it adds another toolchain and does not naturally provide a
  C-compatible `realloc`/`free` ABI without wrapper work. Treat as a fallback.
- `mimalloc`: probably not worth starting with for a single-threaded,
  freestanding Wasm plugin. It is more complex and commonly discussed in
  Emscripten/WASI contexts.

Required behavior:

- `malloc`, `calloc`, `realloc`, `free`, and aligned C++ `new/delete` work
  consistently.
- Out-of-memory traps or returns `nullptr` in a controlled way.
- No unchecked writes past the fixed Wasm memory.
- Repeated format calls do not monotonically consume all memory for ordinary
  inputs.

## 3. Minimal integration tests

Do not unit test clang-format. Test only the behavior owned by this plugin and
the dprint boundary.

Add one test entrypoint, for example `pixi run test`, that builds the Wasm plugin
and runs a Node-based ABI harness plus a few dprint CLI checks.

Suggested cases:

- plugin info parses and has release metadata.
- license text is non-placeholder.
- module has zero imports.
- default C++ formatting changes `int main(){return 1;}`.
- configured style applies, for example `BasedOnStyle: Microsoft`.
- dprint globals map only when not overridden: `lineWidth`, `indentWidth`,
  `useTabs`.
- invalid config produces config diagnostics.
- unsupported filesystem style discovery is rejected or documented:
  `BasedOnStyle: file` and any parent-config inheritance mode should not silently
  pretend to work.
- `format_range` formats only the requested range or returns a clear error for
  bad ranges.
- repeated formatting in one instance does not exhaust memory.
- `check_config_updates` returns the expected wrapper response.

The existing smoke script can be split into:

- `scripts/build_plugin.sh`
- `tests/plugin_abi_test.js`
- `tests/dprint_cli_test.sh`

## 4. Fill out metadata

Current `get_plugin_info()` is hand-written JSON with placeholder values and no
`updateUrl`.

Release metadata should look like:

```json
{
  "name": "dprint-plugin-clang-format",
  "version": "0.1.0",
  "configKey": "clangFormat",
  "fileExtensions": ["c", "cc", "cpp", "cxx", "h", "hh", "hpp", "hxx", "m", "mm"],
  "helpUrl": "https://github.com/<user>/dprint-plugin-clang-format",
  "configSchemaUrl": "https://plugins.dprint.dev/<user>/dprint-plugin-clang-format/0.1.0/schema.json",
  "updateUrl": "https://plugins.dprint.dev/<user>/dprint-plugin-clang-format/latest.json"
}
```

Also replace `get_license_text()` with the real license text. The simplest path
is to add a `LICENSE` file and either embed it at compile time or keep the string
in one generated header.

## 5. Support configuration properly

Current state accepts flat PascalCase clang-format keys and rejects nested JSON
objects. That blocks common clang-format settings such as `IncludeCategories`.

Recommended config model:

- Use `clangFormat` as the dprint config key.
- Accept a JSON representation of clang-format YAML using clang-format option
  names.
- Preserve clang-format's native shape where possible:
  - scalars as JSON scalars
  - YAML sequences as JSON arrays
  - YAML mappings as JSON objects
- Convert JSON to YAML text with a structured emitter, then pass it to
  `clang::format::parseConfiguration`.
- Keep `style` as a legacy alias for `BasedOnStyle` for now, but document it.
- Map dprint globals only when the equivalent clang-format option is absent.

Example:

```json
{
  "clangFormat": {
    "BasedOnStyle": "LLVM",
    "ColumnLimit": 100,
    "IncludeCategories": [
      { "Regex": "^<.*>", "Priority": 1 },
      { "Regex": ".*", "Priority": 2 }
    ]
  }
}
```

Important semantic decisions:

- `BasedOnStyle: file` should produce a config diagnostic. The sandboxed plugin
  cannot discover `.clang-format` files.
- Inherit-parent-config behavior should produce a diagnostic for the same reason.
- Unknown keys can be delegated to `parseConfiguration`, but diagnostics should
  point users at `clangFormat.<key>` where feasible.
- `get_resolved_config()` should return the normalized JSON config the plugin is
  actually using, not an internal-only partial object.

Add `schema.json` once the config shape is settled. It can be permissive at
first because clang-format has many versioned options.

## 6. `check_config_updates`

`check_config_updates` is not the mechanism that discovers GitHub releases. In
dprint schema v4, release discovery comes from `PluginInfo.updateUrl`. The dprint
CLI reads that URL during `dprint config update`, updates the plugin reference,
then calls `check_config_updates` so the upgraded plugin can migrate its own
configuration keys.

The Wasm ABI shape is:

- input: JSON `{"oldVersion": string | null, "config": object}`
- output: JSON response wrapper `{"kind":"ok","data":[...]}` or
  `{"kind":"err","data":"message"}`
- each config change has a path and an add/set/remove operation.

Existing dprint Rust plugins commonly return an empty change list:

```json
{ "kind": "ok", "data": [] }
```

Recommended first release behavior:

- Keep `check_config_updates` implemented.
- Return `{"kind":"ok","data":[]}`.
- Add one ABI test for that exact response.
- Document that there are no config migrations yet.

Future use cases:

- Rename `style` to `BasedOnStyle`.
- Remove deprecated aliases.
- Rewrite old config shapes if the JSON representation changes.

## 7. dprint Wasm APIs we can use

dprint provides imports under the `dprint` Wasm module. A plugin only needs to
declare these imports if it uses them.

Available host APIs in schema v4:

- `host_write_buffer(pointer)` copies host-side bytes into plugin memory.
- `host_format(...)` asks dprint to format a file with another plugin.
- `host_get_formatted_text()` retrieves host-format output.
- `host_get_error_text()` retrieves host-format errors.
- `host_has_cancelled()` lets long-running formatting notice cancellation.

For this plugin:

- Do not use `host_format` for normal C/C++ formatting; LibFormat is the core
  formatter.
- Consider importing `host_has_cancelled` only if LibFormat can be interrupted at
  useful points. If not, importing it is noise.
- Keep zero imports unless a host API creates clear user value. Zero imports are
  currently a useful property and easy to verify.

## 8. CI is deferred

Do not block the first experimental release on CI. Instead, require a manual
release checklist:

```sh
git clean -xfd build third_party
pixi run fetch-llvm
pixi run configure-llvm-wasm
pixi run ninja -C build/llvm-wasm clangFormat
pixi run test
```

Before uploading:

- `wasm-validate build/dprint-clang-format-plugin.wasm` passes.
- import count is zero.
- `dprint fmt --stdin cpp --plugins build/dprint-clang-format-plugin.wasm`
  passes.
- `plugin.wasm` and optional `schema.json` are attached to the GitHub release.
- README install instructions use the final `plugins.dprint.dev` URL shape.

CI can come later as a release hardening step.

## 9. Additional cleanup ideas

- Make LLVM version and plugin version constants single-sourced.
- Rename the repo/package from `spike` terminology to release terminology.
- Add a small `docs/config.md` explaining how JSON maps to clang-format YAML.
- Add a `docs/limitations.md` for unsupported filesystem discovery and any
  language limitations.
- Audit exported symbols and remove exported `malloc`/`free` unless dprint or
  tests need them. The dprint ABI communicates through `clear_shared_bytes` and
  `get_shared_bytes_ptr`.
- Consider stripping the Wasm name section for release if size matters later.

## Release checklist

- [ ] Remove prototype reports and unused scripts.
- [ ] Replace bump allocator with a real allocator.
- [ ] Add integration tests.
- [ ] Fill out plugin metadata and license text.
- [ ] Add `updateUrl`.
- [ ] Implement full JSON-to-clang-format config conversion.
- [ ] Reject or document filesystem-dependent clang-format behavior.
- [ ] Keep `check_config_updates` as empty-ok with a test.
- [ ] Add optional `schema.json`.
- [ ] Update README from spike docs to install/release docs.
