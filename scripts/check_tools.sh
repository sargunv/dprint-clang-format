#!/usr/bin/env bash
set -euo pipefail

clang++ --version | head -1
wasm-ld --version | head -1
cmake --version | head -1
ninja --version
wasm-objdump --version
wasm-validate --version
