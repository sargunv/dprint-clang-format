#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

llvm_src="${LLVM_SRC:-third_party/llvm-project-22.1.7.src}"
llvm_build="${LLVM_WASM_BUILD:-build/llvm-wasm}"
libcxx_include="${LIBCXX_INCLUDE:-$llvm_src/libcxx/include}"
entry_source="${BUILD_ENTRY_SOURCE:?BUILD_ENTRY_SOURCE is required}"
entry_object="${BUILD_ENTRY_OBJECT:?BUILD_ENTRY_OBJECT is required}"
output_wasm="${BUILD_OUTPUT_WASM:?BUILD_OUTPUT_WASM is required}"
exports="${BUILD_EXPORTS:?BUILD_EXPORTS is required}"

if [[ ! -f "$llvm_build/lib/libclangFormat.a" ]]; then
  echo "libclangFormat.a not found at $llvm_build/lib/libclangFormat.a" >&2
  echo "run: pixi run ninja -C $llvm_build clangFormat" >&2
  exit 1
fi

scripts/build_wasm_cxx_shim.sh
source scripts/wasm_cxx_shim_objects.sh

mkdir -p build

common_flags=(
  --target=wasm32-unknown-unknown
  -std=c++17
  -Os
  -ffreestanding
  -fno-exceptions
  -fno-rtti
  -fno-threadsafe-statics
  -fvisibility=hidden
  -ffunction-sections
  -fdata-sections
  -nostdinc++
  -DCLANG_BUILD_STATIC=1
  -D_FILE_OFFSET_BITS=64
  -D_LARGEFILE_SOURCE
  -D__STDC_CONSTANT_MACROS
  -D__STDC_FORMAT_MACROS
  -D__STDC_LIMIT_MACROS
  -D_LIBCPP_DISABLE_EXTERN_TEMPLATE
  -isystem support/libcxx-wasm/include
  -isystem "$libcxx_include"
  -isystem "$llvm_src/libcxx/src/include"
  -isystem support/wasm-sysroot/include
  -I "$llvm_src/clang/include"
  -I "$llvm_build/tools/clang/include"
  -I "$llvm_build/include"
  -I "$llvm_src/llvm/include"
)

clang++ "${common_flags[@]}" -c "$entry_source" -o "$entry_object"
clang++ "${common_flags[@]}" -c src/wasm_support.cpp -o build/wasm_support.o

libcxx_objects=(
  build/libcxx_chrono.o
  build/libcxx_error_category.o
  build/libcxx_functional.o
  build/libcxx_hash.o
  build/libcxx_memory.o
  build/libcxx_new_helpers.o
  build/libcxx_string.o
  build/libcxx_system_error.o
)

clang++ "${common_flags[@]}" -D_LIBCPP_BUILDING_LIBRARY -c "$llvm_src/libcxx/src/chrono.cpp" -o build/libcxx_chrono.o
clang++ "${common_flags[@]}" -D_LIBCPP_BUILDING_LIBRARY -c "$llvm_src/libcxx/src/error_category.cpp" -o build/libcxx_error_category.o
clang++ "${common_flags[@]}" -D_LIBCPP_BUILDING_LIBRARY -c "$llvm_src/libcxx/src/functional.cpp" -o build/libcxx_functional.o
clang++ "${common_flags[@]}" -D_LIBCPP_BUILDING_LIBRARY -c "$llvm_src/libcxx/src/hash.cpp" -o build/libcxx_hash.o
clang++ "${common_flags[@]}" -D_LIBCPP_BUILDING_LIBRARY -c "$llvm_src/libcxx/src/memory.cpp" -o build/libcxx_memory.o
clang++ "${common_flags[@]}" -D_LIBCPP_BUILDING_LIBRARY -c "$llvm_src/libcxx/src/new_helpers.cpp" -o build/libcxx_new_helpers.o
clang++ "${common_flags[@]}" -D_LIBCPP_BUILDING_LIBRARY -c "$llvm_src/libcxx/src/string.cpp" -o build/libcxx_string.o
clang++ "${common_flags[@]}" -std=c++20 -D_LIBCPP_BUILDING_LIBRARY -c "$llvm_src/libcxx/src/system_error.cpp" -o build/libcxx_system_error.o

static_libs=()
while IFS= read -r lib_path; do
  static_libs+=("$lib_path")
done < <(find "$llvm_build/lib" -name 'libclang*.a' | sort)
while IFS= read -r lib_path; do
  static_libs+=("$lib_path")
done < <(find "$llvm_build/lib" -name 'libLLVM*.a' | sort)

link_libs=()
for _ in 1 2 3 4; do
  link_libs+=("${static_libs[@]}")
done

link_exports=()
for export_name in $exports; do
  link_exports+=("-Wl,--export=$export_name")
done

clang++ \
  --target=wasm32-unknown-unknown \
  -nostdlib \
  -Wl,--no-entry \
  -Wl,--initial-memory=67108864 \
  -Wl,--max-memory=67108864 \
  "${link_exports[@]}" \
  -Wl,--export=malloc \
  -Wl,--export=free \
  -Wl,--export-memory \
  -Wl,--gc-sections \
  "$entry_object" \
  build/wasm_support.o \
  "${wasm_cxx_shim_objects[@]}" \
  "${libcxx_objects[@]}" \
  "${link_libs[@]}" \
  -o "$output_wasm"

wasm-validate "$output_wasm"
ls -lh "$output_wasm"
