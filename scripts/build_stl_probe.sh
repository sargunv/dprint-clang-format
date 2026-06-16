#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

libcxx_include="${LIBCXX_INCLUDE:-third_party/llvm-project-22.1.7.src/libcxx/include}"

if [[ ! -f "$libcxx_include/string" ]]; then
  echo "libc++ headers not found at $libcxx_include" >&2
  echo "run: pixi run fetch-llvm" >&2
  exit 1
fi

mkdir -p build

common_flags=(
  --target=wasm32-unknown-unknown
  -std=c++20
  -Oz
  -ffreestanding
  -fno-exceptions
  -fno-rtti
  -fno-threadsafe-statics
  -fvisibility=hidden
  -nostdinc++
  -D_LIBCPP_DISABLE_EXTERN_TEMPLATE
  -isystem support/libcxx-wasm/include
  -isystem "$libcxx_include"
  -isystem third_party/llvm-project-22.1.7.src/libcxx/src/include
  -isystem support/wasm-sysroot/include
)

clang++ "${common_flags[@]}" -c src/stl_probe.cpp -o build/stl_probe.o
clang++ "${common_flags[@]}" -c src/wasm_support.cpp -o build/wasm_support.o
clang++ "${common_flags[@]}" -D_LIBCPP_BUILDING_LIBRARY -c third_party/llvm-project-22.1.7.src/libcxx/src/string.cpp -o build/libcxx_string.o

clang++ \
  --target=wasm32-unknown-unknown \
  -nostdlib \
  -Wl,--no-entry \
  -Wl,--export=stl_probe \
  -Wl,--export=malloc \
  -Wl,--export=free \
  -Wl,--export-memory \
  -Wl,--strip-all \
  build/stl_probe.o \
  build/wasm_support.o \
  build/libcxx_string.o \
  -o build/stl-probe.wasm

wasm-validate build/stl-probe.wasm
ls -lh build/stl-probe.wasm
