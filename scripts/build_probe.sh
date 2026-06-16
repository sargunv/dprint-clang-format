#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

mkdir -p build

clang++ \
  --target=wasm32-unknown-unknown \
  -std=c++20 \
  -Oz \
  -ffreestanding \
  -fno-exceptions \
  -fno-rtti \
  -fno-threadsafe-statics \
  -fvisibility=hidden \
  -nostdlib \
  -Wl,--no-entry \
  -Wl,--export=probe_version \
  -Wl,--export=format_probe \
  -Wl,--export-memory \
  -Wl,--strip-all \
  src/probe_freestanding.cpp \
  -o build/probe.wasm

wasm-validate build/probe.wasm
ls -lh build/probe.wasm
