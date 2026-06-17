#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

scripts/check_tools.sh
scripts/fetch_wasm_cxx_shim.sh
scripts/build_probe.sh
scripts/build_stl_probe.sh
scripts/report_wasm_imports.sh build/stl-probe.wasm reports/stl-probe-imports.md
scripts/report_wasm_imports.sh build/probe.wasm reports/probe-imports.md
