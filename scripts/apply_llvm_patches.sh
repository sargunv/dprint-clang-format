#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

llvm_version="${LLVM_VERSION:-22.1.7}"
source_dir="third_party/llvm-project-${llvm_version}.src"

if [[ ! -f "$source_dir/clang/lib/Format/Format.cpp" ]]; then
  echo "LLVM source not found at $source_dir" >&2
  echo "run: pixi run fetch-llvm" >&2
  exit 1
fi

apply_patch() {
  local patch_file="$1"
  local marker_file="$2"
  local marker_pattern="$3"

  if [[ ! -f "$patch_file" ]]; then
    echo "patch not found: $patch_file" >&2
    exit 1
  fi

  if rg -q "$marker_pattern" "$marker_file"; then
    echo "already applied: $(basename "$patch_file")"
    return 0
  fi

  cd "$source_dir"
  patch -p1 -N --forward < "$patch_file"
  cd "$repo_root"
  echo "applied $(basename "$patch_file")"
}

apply_patch \
  "$repo_root/support/patches/dprint-wasm-no-real-fs.patch" \
  "$source_dir/clang/lib/Format/Format.cpp" \
  'filesystem style discovery is unavailable on wasm'

apply_patch \
  "$repo_root/support/patches/dprint-wasm-skip-frontend-openmp.patch" \
  "$source_dir/clang/lib/Basic/CMakeLists.txt" \
  'LibFormat wasm cross-build does not need the OpenMP frontend library'

apply_patch \
  "$repo_root/support/patches/dprint-wasm-fixed-date-time.patch" \
  "$source_dir/clang/lib/Lex/PPMacroExpansion.cpp" \
  'Jan  1 1970'

apply_patch \
  "$repo_root/support/patches/dprint-wasm-trim-fs-deps.patch" \
  "$source_dir/llvm/lib/Support/Unix/Program.inc" \
  'process execution is unavailable on wasm'

apply_patch \
  "$repo_root/support/patches/dprint-wasm-trim-support-deps.patch" \
  "$source_dir/llvm/lib/Support/raw_socket_stream.cpp" \
  'unix sockets are unavailable on wasm'

apply_patch \
  "$repo_root/support/patches/dprint-wasm-trim-path-posix.patch" \
  "$source_dir/llvm/lib/Support/Path.cpp" \
  'void)ToFD;'

apply_patch \
  "$repo_root/support/patches/dprint-wasm-trim-path-posix-inc.patch" \
  "$source_dir/llvm/lib/Support/Unix/Path.inc" \
  'void)Buf;'

apply_patch \
  "$repo_root/support/patches/dprint-wasm-trim-crash-signals.patch" \
  "$source_dir/llvm/lib/Support/CrashRecoveryContext.cpp" \
  'installExceptionOrSignalHandlers() {}'

apply_patch \
  "$repo_root/support/patches/dprint-wasm-trim-env-path.patch" \
  "$source_dir/llvm/lib/Support/Jobserver.cpp" \
  'return nullptr;'

apply_patch \
  "$repo_root/support/patches/dprint-wasm-trim-path-fs.patch" \
  "$source_dir/llvm/lib/Support/Unix/Path.inc" \
  'disk_space(const Twine &Path)'

