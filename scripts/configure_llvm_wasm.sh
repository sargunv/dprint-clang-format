#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

llvm_version="${LLVM_VERSION:-22.1.7}"
source_root="third_party/llvm-project-${llvm_version}.src"
build_dir="${LLVM_WASM_BUILD_DIR:-build/llvm-wasm}"

if [[ ! -f "$source_root/llvm/CMakeLists.txt" ]]; then
  echo "LLVM source not found at $source_root" >&2
  echo "run: pixi run fetch-llvm" >&2
  exit 1
fi

cmake \
  -S "$source_root/llvm" \
  -B "$build_dir" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCMAKE_SYSTEM_NAME=Generic \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_COMPILER_TARGET=wasm32-unknown-unknown \
  -DCMAKE_CXX_COMPILER_TARGET=wasm32-unknown-unknown \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DCMAKE_C_FLAGS="-ffreestanding -DLLVM_ON_UNIX -DCLANG_BUILD_STATIC=1 -DHAVE_SYSEXITS_H=1 -DHAVE_SYSCONF=1 -DHAVE_GETRUSAGE=1 -DHAVE_SYS_MMAN_H=1 -DHAVE_DLOPEN=1 -isystem $repo_root/support/wasm-sysroot/include" \
  -DCMAKE_CXX_FLAGS="-ffreestanding -DLLVM_ON_UNIX -DCLANG_BUILD_STATIC=1 -DHAVE_SYSEXITS_H=1 -DHAVE_SYSCONF=1 -DHAVE_GETRUSAGE=1 -DHAVE_SYS_MMAN_H=1 -DHAVE_DLOPEN=1 -fno-exceptions -fno-rtti -fno-threadsafe-statics -nostdinc++ -D_LIBCPP_DISABLE_EXTERN_TEMPLATE -isystem $repo_root/support/libcxx-wasm/include -isystem $repo_root/$source_root/libcxx/include -isystem $repo_root/$source_root/libcxx/src/include -isystem $repo_root/support/wasm-sysroot/include" \
  -DLLVM_NATIVE_TOOL_DIR="$repo_root/$build_dir/NATIVE/bin" \
  -DLLVM_HEADERS_TABLEGEN="$repo_root/$build_dir/NATIVE/bin/llvm-min-tblgen" \
  -DLLVM_TABLEGEN="$repo_root/$build_dir/NATIVE/bin/llvm-tblgen" \
  -DCLANG_TABLEGEN="$repo_root/$build_dir/NATIVE/bin/clang-tblgen" \
  -DLLVM_ENABLE_PROJECTS=clang \
  -DLLVM_TARGETS_TO_BUILD="" \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DLLVM_BUILD_TOOLS=OFF \
  -DCLANG_BUILD_TOOLS=OFF \
  -DLLVM_ENABLE_BINDINGS=OFF \
  -DLLVM_ENABLE_ZLIB=OFF \
  -DLLVM_ENABLE_ZSTD=OFF \
  -DLLVM_ENABLE_TERMINFO=OFF \
  -DLLVM_ENABLE_LIBXML2=OFF \
  -DLLVM_ENABLE_THREADS=OFF \
  -DLLVM_ENABLE_PIC=OFF \
  -DLLVM_ENABLE_BACKTRACES=OFF \
  -DLLVM_ENABLE_UNWIND_TABLES=OFF
