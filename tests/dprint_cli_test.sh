#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

plugin_path="$(pwd)/build/plugin.wasm"

formatted="$(
  printf 'int main(){return 1;}\n' |
    dprint fmt --stdin cpp --config-discovery=false --plugins "$plugin_path"
)"

expected="int main() { return 1; }"
if [[ "$formatted" != *"$expected"* ]]; then
  echo "unexpected dprint CLI output: $formatted" >&2
  exit 1
fi

config_path="build/dprint-test.json"
printf '{\n  "plugins": ["%s"],\n  "clangFormat": { "BasedOnStyle": "Microsoft" },\n  "lineWidth": 120,\n  "indentWidth": 4\n}\n' "$plugin_path" > "$config_path"

formatted_with_config="$(
  printf 'void f(){if(true){return;}}\n' |
    dprint fmt --stdin cpp --config "$config_path" --config-discovery=false
)"

if [[ "$formatted_with_config" != *$'void f()\n{\n    if (true)'* ]]; then
  echo "unexpected dprint CLI configured output: $formatted_with_config" >&2
  exit 1
fi

echo "dprint CLI tests passed"
