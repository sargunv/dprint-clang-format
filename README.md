# dprint-clang-format

This is a port of [clang-format](https://clang.llvm.org/docs/ClangFormat.html)
to a [dprint](https://dprint.dev) Wasm plugin. It can be used to format
C/C++/Obj-C source with dprint.

## Setup

Add the latest plugin release to your `dprint.json`:

```jsonc
{
  "$schema": "https://dprint.dev/schemas/v0.json",
  "plugins": [
    "https://plugins.dprint.dev/sargunv/dprint-clang-format/latest.json"
  ]
}
```

Then run dprint normally:

```sh
dprint fmt
```

The plugin formats files with these extensions:

```text
c cc cpp cxx h hh hpp hxx m mm
```

## Configure

clang-format options go under the `clangFormat` config key. Use clang-format
option names represented as JSON:

```jsonc
{
  "$schema": "https://dprint.dev/schemas/v0.json",
  "plugins": ["./build/plugin.wasm"],
  "clangFormat": {
    "BasedOnStyle": "LLVM",
    "ColumnLimit": 100,
    "IndentWidth": 2,
    "SpaceBeforeParens": "Custom",
    "SpaceBeforeParensOptions": {
      "AfterControlStatements": false,
      "AfterFunctionDeclarationName": true,
      "AfterFunctionDefinitionName": true
    }
  }
}
```

dprint global options are also mapped when the matching clang-format option is
not set explicitly:

| dprint option | clang-format option |
| ------------- | ------------------- |
| `lineWidth`   | `ColumnLimit`       |
| `indentWidth` | `IndentWidth`       |
| `useTabs`     | `UseTab`            |

`clangFormat` wins over the global option when both are present.

Filesystem-dependent style discovery is unsupported in the Wasm sandbox, so
`BasedOnStyle: "file"` and `BasedOnStyle: "InheritParentConfig"` are rejected.
Put the style in dprint config instead of relying on `.clang-format` discovery.

## How it works

The plugin links Clang LibFormat into a dprint plugin ABI v4 WebAssembly module.
It is built for `wasm32-unknown-unknown` with no Wasm imports.

We patch LLVM to disable code paths that do not make sense in a sandboxed dprint
plugin, such as real filesystem probing, process/environment helpers,
crash/signal handling, and other cold-path host facilities.
