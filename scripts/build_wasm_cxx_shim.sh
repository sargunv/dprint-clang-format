#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

version="${WASM_CXX_SHIM_VERSION:-0.5.0}"
shim_root="${WASM_CXX_SHIM_ROOT:-third_party/wasm-cxx-shim-${version}}"
out_dir="${WASM_CXX_SHIM_BUILD:-build/wasm-cxx-shim}"

if [[ ! -f "$shim_root/CMakeLists.txt" ]]; then
  echo "wasm-cxx-shim source not found at $shim_root" >&2
  echo "run: pixi run fetch-wasm-cxx-shim" >&2
  exit 1
fi

mkdir -p "$out_dir"

target_flags=(
  --target=wasm32-unknown-unknown
  -ffreestanding
  -Os
  -fvisibility=hidden
)

compile_libc() {
  local common=(
    "${target_flags[@]}"
    -fno-builtin
    -fno-strict-aliasing
    -DBULK_MEMORY_THRESHOLD=200
    -Wno-null-pointer-arithmetic
    -Wno-unused-but-set-variable
    -Wno-expansion-to-defined
    -Wno-pointer-sign
    -I "$shim_root/libc/include"
  )

  clang "${common[@]}" -c "$shim_root/libc/src/dlmalloc/dlmalloc.c" -o "$out_dir/libc_dlmalloc.o"
  clang "${common[@]}" -c "$shim_root/libc/src/dlmalloc/sbrk.c" -o "$out_dir/libc_sbrk.o"
  clang "${common[@]}" -c "$shim_root/libc/src/musl/memcpy.c" -o "$out_dir/libc_memcpy.o"
  clang "${common[@]}" -c "$shim_root/libc/src/musl/memmove.c" -o "$out_dir/libc_memmove.o"
  clang "${common[@]}" -c "$shim_root/libc/src/musl/memset.c" -o "$out_dir/libc_memset.o"
  clang "${common[@]}" -c "$shim_root/libc/src/musl/memcmp.c" -o "$out_dir/libc_memcmp.o"
  clang "${common[@]}" -c "$shim_root/libc/src/musl/strlen.c" -o "$out_dir/libc_strlen.o"
}

compile_libm() {
  local common=(
    "${target_flags[@]}"
    -ffp-contract=off
    -fno-math-errno
    -fno-trapping-math
    -Wno-bitwise-op-parentheses
    -Wno-dangling-else
    -Wno-ignored-attributes
    -Wno-ignored-pragmas
    -Wno-logical-op-parentheses
    -Wno-macro-redefined
    -Wno-missing-braces
    -Wno-parentheses
    -Wno-shift-op-parentheses
    -Wno-sign-compare
    -Wno-string-plus-int
    -Wno-unknown-pragmas
    -Wno-unused-but-set-variable
    -Wno-unused-function
    -Wno-unused-parameter
    -Wno-unused-variable
    -I "$shim_root/libm/src/internal"
    -I "$shim_root/libm/src/musl"
    -I "$shim_root/libm/include"
  )

  local libm_sources=(
    __cos.c __sin.c __tan.c
    __rem_pio2.c __rem_pio2_large.c
    __math_divzero.c __math_invalid.c __math_oflow.c
    __math_uflow.c __math_xflow.c
    acos.c asin.c atan.c atan2.c
    ceil.c copysign.c cos.c sin.c tan.c
    exp.c exp_data.c
    fabs.c floor.c fma.c
    fmax.c fmin.c
    hypot.c ilogb.c ldexp.c
    log.c log_data.c log10.c
    log2.c log2_data.c
    pow.c pow_data.c
    remquo.c round.c scalbn.c
    sqrt.c sqrt_data.c trunc.c
  )

  for source in "${libm_sources[@]}"; do
    local object_name="libm_${source%.c}.o"
    clang "${common[@]}" -c "$shim_root/libm/src/musl/$source" -o "$out_dir/$object_name"
  done
}

compile_libcxx() {
  local common=(
    "${target_flags[@]}"
    -std=c++17
    -fno-exceptions
    -fno-rtti
  )

  clang++ "${common[@]}" -c "$shim_root/libcxx/src/cxa.cpp" -o "$out_dir/libcxx_cxa.o"
  clang++ "${common[@]}" -c "$shim_root/libcxx/src/operator_new_delete.cpp" -o "$out_dir/libcxx_operator_new_delete.o"
  clang++ "${common[@]}" -c "$shim_root/libcxx/src/verbose_abort.cpp" -o "$out_dir/libcxx_verbose_abort.o"
  clang++ "${common[@]}" -c "$shim_root/libcxx/src/exception.cpp" -o "$out_dir/libcxx_exception.o"
  clang++ "${common[@]}" -c "$shim_root/libcxx/src/stubs.cpp" -o "$out_dir/libcxx_stubs.o"
}

compile_libc
compile_libm
compile_libcxx

echo "Built wasm-cxx-shim objects in $out_dir"
