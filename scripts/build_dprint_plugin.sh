#!/usr/bin/env bash
set -euo pipefail

BUILD_ENTRY_SOURCE=src/dprint_plugin.cpp \
BUILD_ENTRY_OBJECT=build/dprint_plugin.o \
BUILD_OUTPUT_WASM=build/dprint-clang-format-plugin.wasm \
BUILD_EXPORTS="dprint_plugin_version_4 get_shared_bytes_ptr clear_shared_bytes get_plugin_info get_license_text register_config release_config get_config_diagnostics get_resolved_config get_config_file_matching set_file_path set_override_config format format_range get_formatted_text get_error_text check_config_updates" \
scripts/link_libformat_wasm.sh
