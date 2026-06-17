#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
  echo "usage: $0 <module.wasm> <report.md>" >&2
  exit 2
fi

wasm_path="$1"
report_path="$2"

if [[ ! -f "$wasm_path" ]]; then
  echo "wasm module not found: $wasm_path" >&2
  exit 1
fi

mkdir -p "$(dirname "$report_path")"

tmp_objdump="$(mktemp)"
trap 'rm -f "$tmp_objdump"' EXIT
wasm-objdump -x "$wasm_path" > "$tmp_objdump"

import_count="$(
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
  ' "$tmp_objdump"
)"

{
  echo "# Wasm Import Report"
  echo
  echo "- Module: \`$wasm_path\`"
  echo "- Generated: \`$(date -u '+%Y-%m-%dT%H:%M:%SZ')\`"
  echo "- Import count: \`$import_count\`"
  echo
  echo "## Import Section"
  echo
  if [[ "$import_count" == "0" ]]; then
    echo "No imports."
  else
    awk '
      /^Import\[/ { in_imports = 1; print; next }
      in_imports && /^[A-Z][A-Za-z]*\[/ { in_imports = 0 }
      in_imports { print }
    ' "$tmp_objdump"
  fi
} > "$report_path"

echo "wrote $report_path"
