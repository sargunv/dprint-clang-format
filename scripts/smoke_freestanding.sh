#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

scripts/check_tools.sh
scripts/build_probe.sh
scripts/report_wasm_imports.sh build/probe.wasm reports/probe-imports.md
