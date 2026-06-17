#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

llvm_version="${LLVM_VERSION:-22.1.7}"
source_dir="third_party/llvm-project-${llvm_version}.src"
patch_file="$repo_root/support/patches/dprint-wasm-no-real-fs.patch"

if [[ ! -f "$source_dir/clang/lib/Format/Format.cpp" ]]; then
  echo "LLVM source not found at $source_dir" >&2
  echo "run: pixi run fetch-llvm" >&2
  exit 1
fi

if [[ ! -f "$patch_file" ]]; then
  echo "patch not found: $patch_file" >&2
  exit 1
fi

if rg -q 'filesystem style discovery is unavailable on wasm' "$source_dir/clang/lib/Format/Format.cpp" &&
   rg -q 'makeIntrusiveRefCnt<llvm::vfs::InMemoryFileSystem>' "$source_dir/clang/lib/Basic/FileManager.cpp" &&
   rg -q 'operation_not_supported' "$source_dir/llvm/lib/Support/raw_ostream.cpp" &&
   rg -q -F '(void)FD;' "$source_dir/llvm/lib/Support/Unix/Process.inc" &&
   rg -q -F '(void)result;' "$source_dir/llvm/lib/Support/Unix/Path.inc"; then
  echo "patches already applied"
  exit 0
fi

cd "$source_dir"
patch -p1 -N --forward < "$patch_file"

echo "applied $patch_file"
