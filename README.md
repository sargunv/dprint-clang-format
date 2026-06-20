# dprint-clang-format

This is a port of [clang-format](https://clang.llvm.org/docs/ClangFormat.html)
to a [dprint](https://dprint.dev) Wasm plugin. It can be used to format
C/C++/Obj-C source with dprint.

## Setup

Add the latest plugin release to your `dprint.json`:

```sh
dprint add sargunv/dprint-clang-format
```

Then run dprint normally:

```sh
dprint fmt
```

The plugin formats files with these extensions: `.c`, `.cc`, `.cpp`, `.cxx`,
`.h`, `.hh`, `.hpp`, `.hxx`, `.m`, `.mm`.

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

## Limitations

Filesystem-dependent style discovery is unsupported in the Wasm sandbox, so
`BasedOnStyle: "file"` and `BasedOnStyle: "InheritParentConfig"` are rejected.
Put the style in dprint config instead of relying on `.clang-format` discovery.

Files whose nested `#if`/`#ifdef`/`#elif` branches produce more than 16 branch
combinations may format differently from native clang-format in inactive or
platform-specific branches.

## How it works

The plugin links Clang LibFormat into a dprint plugin ABI v4 WebAssembly module.
It is built for `wasm32-unknown-unknown` with no Wasm imports.

We patch LLVM to disable code paths that do not make sense in a sandboxed dprint
plugin, such as real filesystem probing, process/environment helpers,
crash/signal handling, and other cold-path host facilities.

## Performance

I expected this to have some overhead compared to native clang-format, but it
appears to be about 30% faster for some reason. Try it yourself with
`mise run bench`.

```txt
Benchmarking stdin formatting over 250 LLVM/Clang source files.
Set BENCH_FILE_LIMIT=all for the full LLVM tree, or set BENCH_RUNS/BENCH_WARMUP to adjust rounds.
Benchmark 1: native clang-format
  Time (mean ± σ):      6.469 s ±  0.095 s    [User: 4.691 s, System: 1.502 s]
  Range (min … max):    6.385 s …  6.573 s    3 runs

Benchmark 2: dprint wasm plugin
  Time (mean ± σ):      4.969 s ±  0.094 s    [User: 2.986 s, System: 1.674 s]
  Range (min … max):    4.913 s …  5.078 s    3 runs

  Warning: Statistical outliers were detected. Consider re-running this benchmark on a quiet system without any interferences from other programs. It might help to use the '--warmup' or '--prepare' options.

Summary
  dprint wasm plugin ran
    1.30 ± 0.03 times faster than native clang-format
```

Tested on MacBook Pro 14" with an M5 Pro.
