setup() {
  repo_root="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
  cd "$repo_root"
  plugin_path="$repo_root/build/plugin.wasm"
}

format_stdin() {
  local source_text="$1"
  shift

  printf '%s\n' "$source_text" |
    dprint fmt --stdin cpp --config-discovery=false "$@"
}

@test "plugin wasm has zero imports" {
  run pixi run wasm-objdump -x "$plugin_path"

  [ "$status" -eq 0 ]
  [[ "$output" != *"Import["* ]]
}

@test "plugin ABI behaves as expected" {
  run node tests/plugin_abi_test.js "$plugin_path"

  [ "$status" -eq 0 ]
  [[ "$output" == *"plugin ABI tests passed"* ]]
}

@test "dprint CLI formats stdin with default config" {
  run format_stdin "int main(){return 1;}" --plugins "$plugin_path"

  [ "$status" -eq 0 ]
  [[ "$output" == *"int main() { return 1; }"* ]]
}

@test "dprint CLI applies clangFormat config" {
  config_path="$BATS_TEST_TMPDIR/dprint.json"
  cat > "$config_path" <<JSON
{
  "plugins": ["$plugin_path"],
  "clangFormat": { "BasedOnStyle": "Microsoft" },
  "lineWidth": 120,
  "indentWidth": 4
}
JSON

  run format_stdin "void f(){if(true){return;}}" --config "$config_path"

  [ "$status" -eq 0 ]
  [[ "$output" == *$'void f()\n{\n    if (true)'* ]]
}
