setup() {
  repo_root="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
  cd "$repo_root"
  plugin_path="$repo_root/build/plugin.wasm"
}

format_file_with_dprint() {
  local config_path="$1"
  local source_path="$2"

  dprint fmt --stdin "$source_path" --config "$config_path" --config-discovery=false < "$source_path"
}

format_file_with_clang_format() {
  local style="$1"
  local source_path="$2"

  pixi run clang-format --style="$style" "$source_path"
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

@test "dprint CLI matches clang-format on real C++ source with LLVM style" {
  source_path="third_party/llvm-project-22.1.7.src/libcxx/test/libcxx/algorithms/debug_less.pass.cpp"
  config_path="$BATS_TEST_TMPDIR/dprint.json"
  write_config "$config_path" '
  "clangFormat": { "BasedOnStyle": "LLVM" }'

  expected="$(format_file_with_clang_format '{BasedOnStyle: LLVM}' "$source_path")"
  run format_file_with_dprint "$config_path" "$source_path"

  [ "$status" -eq 0 ]
  [ "$output" = "$expected" ]
}

@test "dprint CLI matches clang-format on real C++ source with Microsoft style" {
  source_path="third_party/llvm-project-22.1.7.src/libcxx/test/libcxx/algorithms/alg.sorting/assert.sort.invalid_comparator/assert.sort.invalid_comparator.pass.cpp"
  config_path="$BATS_TEST_TMPDIR/dprint.json"
  write_config "$config_path" '
  "clangFormat": { "BasedOnStyle": "Microsoft" },
  "lineWidth": 120,
  "indentWidth": 4'

  expected="$(format_file_with_clang_format '{BasedOnStyle: Microsoft, ColumnLimit: 120, IndentWidth: 4}' "$source_path")"
  run format_file_with_dprint "$config_path" "$source_path"

  [ "$status" -eq 0 ]
  [ "$output" = "$expected" ]
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
  source_path="third_party/llvm-project-22.1.7.src/libcxx/test/libcxx/algorithms/debug_less.pass.cpp"
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

  expected="$(format_file_with_clang_format '{BasedOnStyle: LLVM, SpaceBeforeParens: Custom, SpaceBeforeParensOptions: {AfterControlStatements: false, AfterFunctionDeclarationName: true, AfterFunctionDefinitionName: true}}' "$source_path")"
  run format_file_with_dprint "$config_path" "$source_path"

  [ "$status" -eq 0 ]
  [ "$output" = "$expected" ]
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
