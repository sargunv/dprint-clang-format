#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

version="${WASM_CXX_SHIM_VERSION:-0.5.0}"
dest="third_party/wasm-cxx-shim-${version}"

if [[ -f "$dest/CMakeLists.txt" ]]; then
  echo "wasm-cxx-shim already present at $dest"
  exit 0
fi

mkdir -p third_party
archive="third_party/wasm-cxx-shim-${version}.tar.gz"
url="https://github.com/zmerlynn/wasm-cxx-shim/archive/refs/tags/v${version}.tar.gz"

echo "Fetching wasm-cxx-shim v${version}..."
curl -fsSL "$url" -o "$archive"
tar -xzf "$archive" -C third_party
rm -f "$archive"

extracted="$(find third_party -maxdepth 1 -type d -name 'wasm-cxx-shim-*' ! -path "$dest" | head -1)"
if [[ -n "$extracted" && "$extracted" != "$dest" ]]; then
  rm -rf "$dest"
  mv "$extracted" "$dest"
fi

if [[ ! -f "$dest/CMakeLists.txt" ]]; then
  echo "failed to unpack wasm-cxx-shim to $dest" >&2
  exit 1
fi

echo "wasm-cxx-shim v${version} fetched to $dest"
