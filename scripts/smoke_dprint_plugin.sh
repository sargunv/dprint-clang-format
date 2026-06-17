#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

scripts/link_libformat_wasm.sh
scripts/report_wasm_imports.sh build/dprint-clang-format-plugin.wasm reports/dprint-plugin-imports.md

node <<'JS'
const fs = require("fs");
const bytes = fs.readFileSync("build/dprint-clang-format-plugin.wasm");

WebAssembly.instantiate(bytes, {}).then(({ instance }) => {
  const exports = instance.exports;
  const encoder = new TextEncoder();
  const decoder = new TextDecoder();

  function writeShared(text) {
    const bytes = encoder.encode(text);
    const pointer = exports.clear_shared_bytes(bytes.length);
    new Uint8Array(exports.memory.buffer, pointer, bytes.length).set(bytes);
  }

  function readShared(length) {
    const pointer = exports.get_shared_bytes_ptr();
    return decoder.decode(new Uint8Array(exports.memory.buffer, pointer, length));
  }

  if (exports.dprint_plugin_version_4() !== 4) {
    throw new Error("unexpected dprint schema version");
  }

  const infoLength = exports.get_plugin_info();
  const info = JSON.parse(readShared(infoLength));
  if (info.configKey !== "clangFormat") {
    throw new Error(`unexpected config key: ${info.configKey}`);
  }

  writeShared("{\"plugin\":{\"BasedOnStyle\":\"LLVM\"},\"global\":{}}");
  exports.register_config(1);

  const resolvedLength = exports.get_resolved_config(1);
  const resolved = JSON.parse(readShared(resolvedLength));
  if (resolved.BasedOnStyle !== "LLVM") {
    throw new Error(`unexpected resolved BasedOnStyle: ${JSON.stringify(resolved)}`);
  }

  writeShared("test.cpp");
  exports.set_file_path();
  writeShared("int main(){return 1;}\n");

  const status = exports.format(1);
  if (status !== 1) {
    throw new Error(`expected changed format status, got ${status}`);
  }

  const formattedLength = exports.get_formatted_text();
  const formatted = readShared(formattedLength);
  if (formatted !== "int main() { return 1; }\n") {
    throw new Error(`unexpected formatted text: ${JSON.stringify(formatted)}`);
  }

  console.log(JSON.stringify({ configKey: info.configKey, basedOnStyle: resolved.BasedOnStyle, status, formatted }));
}).catch(error => {
  console.error(error);
  process.exit(1);
});
JS

formatted="$(
  printf 'int main(){return 1;}\n' |
    dprint fmt --stdin cpp --config-discovery=false --plugins build/dprint-clang-format-plugin.wasm
)"

expected="int main() { return 1; }"
if [[ "$formatted" != *"$expected"* ]]; then
  echo "unexpected dprint CLI output: $formatted" >&2
  exit 1
fi

echo "dprint CLI formatted stdin successfully"

plugin_path="$(pwd)/build/dprint-clang-format-plugin.wasm"
config_path="build/dprint-smoke.json"
printf '{\n  "plugins": ["%s"],\n  "clangFormat": { "BasedOnStyle": "Microsoft" },\n  "lineWidth": 120,\n  "indentWidth": 4\n}\n' "$plugin_path" > "$config_path"

formatted_with_config="$(
  printf 'void f(){if(true){return;}}\n' |
    dprint fmt --stdin cpp --config "$config_path" --config-discovery=false
)"

if [[ "$formatted_with_config" != *$'void f()\n{\n    if (true)'* ]]; then
  echo "unexpected dprint CLI configured output: $formatted_with_config" >&2
  exit 1
fi

echo "dprint CLI applied inline style config successfully"
