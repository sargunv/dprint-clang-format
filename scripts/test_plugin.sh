#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

cmake -S . -B build/plugin -G Ninja \
  -DCMAKE_MAKE_PROGRAM="$(command -v ninja)" \
  -DCMAKE_TOOLCHAIN_FILE=cmake/wasm32-unknown-unknown.cmake
cmake --build build/plugin --target plugin

import_count="$(
  wasm-objdump -x build/plugin.wasm |
    awk '
      /^Import\[/ {
        value = $0
        sub(/^Import\[/, "", value)
        sub(/\].*$/, "", value)
        print value
        found = 1
      }
      END {
        if (!found) print "0"
      }
    '
)"

if [[ "$import_count" != "0" ]]; then
  echo "expected zero wasm imports, found $import_count" >&2
  wasm-objdump -x build/plugin.wasm >&2
  exit 1
fi

node tests/plugin_abi_test.js build/plugin.wasm
bash tests/dprint_cli_test.sh
