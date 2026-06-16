#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

llvm_version="${LLVM_VERSION:-22.1.7}"
archive_name="llvm-project-${llvm_version}.src.tar.xz"
release_tag="llvmorg-${llvm_version}"
url="https://github.com/llvm/llvm-project/releases/download/${release_tag}/${archive_name}"

cache_dir="third_party/cache"
source_dir="third_party/llvm-project-${llvm_version}.src"
archive_path="${cache_dir}/${archive_name}"
sha_path="${archive_path}.sha256"

mkdir -p "$cache_dir" third_party

if [[ ! -f "$archive_path" ]]; then
  curl --fail --location --retry 3 --output "$archive_path" "$url"
fi

shasum -a 256 "$archive_path" | tee "$sha_path"

if [[ ! -d "$source_dir" ]]; then
  tar -C third_party -xf "$archive_path"
fi

test -f "$source_dir/llvm/CMakeLists.txt"
test -f "$source_dir/clang/lib/Format/Format.cpp"

echo "LLVM source ready: $source_dir"
