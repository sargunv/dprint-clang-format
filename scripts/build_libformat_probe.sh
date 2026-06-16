#!/usr/bin/env bash
set -euo pipefail

BUILD_ENTRY_SOURCE=src/libformat_probe.cpp \
BUILD_ENTRY_OBJECT=build/libformat_probe.o \
BUILD_OUTPUT_WASM=build/libformat-probe.wasm \
BUILD_EXPORTS=clang_format_probe \
scripts/link_libformat_wasm.sh
