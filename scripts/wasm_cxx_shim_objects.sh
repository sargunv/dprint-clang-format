#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
out_dir="${WASM_CXX_SHIM_BUILD:-$repo_root/build/wasm-cxx-shim}"

wasm_cxx_shim_objects=(
  "$out_dir/libc_dlmalloc.o"
  "$out_dir/libc_sbrk.o"
  "$out_dir/libc_memcpy.o"
  "$out_dir/libc_memmove.o"
  "$out_dir/libc_memset.o"
  "$out_dir/libc_memcmp.o"
  "$out_dir/libc_strlen.o"
  "$out_dir/libcxx_cxa.o"
  "$out_dir/libcxx_operator_new_delete.o"
  "$out_dir/libcxx_verbose_abort.o"
  "$out_dir/libcxx_exception.o"
  "$out_dir/libcxx_stubs.o"
)

libm_sources=(
  __cos __sin __tan
  __rem_pio2 __rem_pio2_large
  __math_divzero __math_invalid __math_oflow
  __math_uflow __math_xflow
  acos asin atan atan2
  ceil copysign cos sin tan
  exp exp_data
  fabs floor fma
  fmax fmin
  hypot ilogb ldexp
  log log_data log10
  log2 log2_data
  pow pow_data
  remquo round scalbn
  sqrt sqrt_data trunc
)

for source in "${libm_sources[@]}"; do
  wasm_cxx_shim_objects+=("$out_dir/libm_${source}.o")
done
