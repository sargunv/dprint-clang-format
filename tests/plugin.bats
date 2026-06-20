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

write_config() {
  local config_path="$1"
  local body="$2"

  cat > "$config_path" <<JSON
{
  "plugins": ["$plugin_path"],
  $body
}
JSON
}

@test "plugin wasm has zero imports" {
  run pixi run wasm-objdump -x "$plugin_path"

  [ "$status" -eq 0 ]
  [[ "$output" != *"Import["* ]]
}

@test "dprint CLI formats stdin with default config" {
  run format_stdin "int main(){return 1;}" --plugins "$plugin_path"

  [ "$status" -eq 0 ]
  [[ "$output" == *"int main() { return 1; }"* ]]
}

@test "dprint CLI applies clangFormat config" {
  config_path="$BATS_TEST_TMPDIR/dprint.json"
  write_config "$config_path" '
  "clangFormat": { "BasedOnStyle": "Microsoft" },
  "lineWidth": 120,
  "indentWidth": 4'

  run format_stdin "void f(){if(true){return;}}" --config "$config_path"

  [ "$status" -eq 0 ]
  [[ "$output" == *$'void f()\n{\n    if (true)'* ]]
}

@test "dprint CLI resolves global options into clangFormat config" {
  config_path="$BATS_TEST_TMPDIR/dprint.json"
  write_config "$config_path" '
  "lineWidth": 20,
  "indentWidth": 3,
  "useTabs": false,
  "clangFormat": {}'

  run dprint output-resolved-config --config "$config_path" --config-discovery=false

  [ "$status" -eq 0 ]
  [[ "$output" == *'"ColumnLimit": 20'* ]]
  [[ "$output" == *'"IndentWidth": 3'* ]]
  [[ "$output" == *'"UseTab": "Never"'* ]]
}

@test "dprint CLI lets clangFormat config override globals" {
  config_path="$BATS_TEST_TMPDIR/dprint.json"
  write_config "$config_path" '
  "lineWidth": 20,
  "clangFormat": { "BasedOnStyle": "LLVM", "ColumnLimit": 80 }'

  run dprint output-resolved-config --config "$config_path" --config-discovery=false

  [ "$status" -eq 0 ]
  [[ "$output" == *'"ColumnLimit": 80'* ]]
}

@test "dprint CLI accepts nested clangFormat config" {
  config_path="$BATS_TEST_TMPDIR/dprint.json"
  write_config "$config_path" '
  "clangFormat": {
    "BasedOnStyle": "LLVM",
    "SpaceBeforeParens": "Custom",
    "SpaceBeforeParensOptions": {
      "AfterControlStatements": false,
      "AfterFunctionDeclarationName": true,
      "AfterFunctionDefinitionName": true
    }
  }'

  run format_stdin "void f(){if(true){return;}}" --config "$config_path"

  [ "$status" -eq 0 ]
  [[ "$output" == *"void f ()"* ]]
  [[ "$output" == *"if(true)"* ]]
}

@test "dprint CLI rejects unsupported clangFormat options" {
  config_path="$BATS_TEST_TMPDIR/dprint.json"
  write_config "$config_path" '
  "clangFormat": { "NotAClangFormatOption": true }'

  run dprint output-resolved-config --config "$config_path" --config-discovery=false

  [ "$status" -eq 1 ]
  [[ "$output" == *"unsupported (clangFormat)"* ]]
}

@test "dprint CLI rejects filesystem-dependent style discovery" {
  config_path="$BATS_TEST_TMPDIR/dprint.json"
  write_config "$config_path" '
  "clangFormat": { "BasedOnStyle": "file" }'

  run dprint output-resolved-config --config "$config_path" --config-discovery=false

  [ "$status" -eq 1 ]
  [[ "$output" == *"Filesystem-dependent clang-format style discovery is not supported (BasedOnStyle)"* ]]
}
