const assert = require("assert");
const fs = require("fs");
const path = require("path");

const wasmPath = process.argv[2] || path.join("build", "plugin.wasm");
const bytes = fs.readFileSync(wasmPath);

const encoder = new TextEncoder();
const decoder = new TextDecoder();

function parseJsonBytes(text) {
  return JSON.parse(text);
}

async function main() {
  const { instance } = await WebAssembly.instantiate(bytes, {});
  const exports = instance.exports;

  function writeShared(text) {
    const bytes = encoder.encode(text);
    const pointer = exports.clear_shared_bytes(bytes.length);
    new Uint8Array(exports.memory.buffer, pointer, bytes.length).set(bytes);
  }

  function readShared(length) {
    const pointer = exports.get_shared_bytes_ptr();
    return decoder.decode(new Uint8Array(exports.memory.buffer, pointer, length));
  }

  function registerConfig(id, plugin, global = {}) {
    writeShared(JSON.stringify({ plugin, global }));
    exports.register_config(id);
  }

  function setFilePath(filePath) {
    writeShared(filePath);
    exports.set_file_path();
  }

  function formatText(configId, filePath, text) {
    setFilePath(filePath);
    writeShared(text);
    const status = exports.format(configId);
    if (status === 0) return { status, text };
    if (status === 1) return { status, text: readShared(exports.get_formatted_text()) };
    return { status, error: readShared(exports.get_error_text()) };
  }

  assert.strictEqual(exports.dprint_plugin_version_4(), 4);

  const info = parseJsonBytes(readShared(exports.get_plugin_info()));
  assert.deepStrictEqual(info, {
    name: "dprint-plugin-clang-format",
    version: "0.1.0",
    configKey: "clangFormat",
    fileExtensions: ["c", "cc", "cpp", "cxx", "h", "hh", "hpp", "hxx", "m", "mm"],
    helpUrl: "https://github.com/sargunv/dprint-clang-format",
    configSchemaUrl: "https://plugins.dprint.dev/sargunv/dprint-clang-format/0.1.0/schema.json",
    updateUrl: "https://plugins.dprint.dev/sargunv/dprint-clang-format/latest.json",
  });

  const license = readShared(exports.get_license_text());
  assert(license.includes("MIT License"));
  assert(license.includes("Copyright"));

  registerConfig(1, { BasedOnStyle: "LLVM" });
  assert.deepStrictEqual(parseJsonBytes(readShared(exports.get_config_diagnostics(1))), []);
  assert.strictEqual(parseJsonBytes(readShared(exports.get_resolved_config(1))).BasedOnStyle, "LLVM");

  const defaultResult = formatText(1, "test.cpp", "int main(){return 1;}\n");
  assert.strictEqual(defaultResult.status, 1);
  assert.strictEqual(defaultResult.text, "int main() { return 1; }\n");

  registerConfig(2, { BasedOnStyle: "Microsoft" }, { lineWidth: 120, indentWidth: 4 });
  const microsoftResult = formatText(2, "test.cpp", "void f(){if(true){return;}}\n");
  assert.strictEqual(microsoftResult.status, 1);
  assert(microsoftResult.text.includes("void f()\n{\n    if (true)"));

  registerConfig(3, {}, { lineWidth: 20, indentWidth: 3, useTabs: false });
  const globalResolved = parseJsonBytes(readShared(exports.get_resolved_config(3)));
  assert.strictEqual(globalResolved.ColumnLimit, 20);
  assert.strictEqual(globalResolved.IndentWidth, 3);
  assert.strictEqual(globalResolved.UseTab, "Never");

  registerConfig(4, { BasedOnStyle: "LLVM", ColumnLimit: 80 }, { lineWidth: 20 });
  assert.strictEqual(parseJsonBytes(readShared(exports.get_resolved_config(4))).ColumnLimit, 80);

  registerConfig(5, {
    BasedOnStyle: "LLVM",
    SpaceBeforeParens: "Custom",
    SpaceBeforeParensOptions: {
      AfterControlStatements: false,
      AfterFunctionDeclarationName: true,
      AfterFunctionDefinitionName: true,
    },
  });
  assert.deepStrictEqual(parseJsonBytes(readShared(exports.get_config_diagnostics(5))), []);
  const nestedResolved = parseJsonBytes(readShared(exports.get_resolved_config(5)));
  assert.strictEqual(nestedResolved.SpaceBeforeParensOptions.AfterFunctionDefinitionName, true);
  const nestedResult = formatText(5, "nested.cpp", "void f(){if(true){return;}}\n");
  assert.strictEqual(nestedResult.status, 1);
  assert(nestedResult.text.includes("void f ()"));
  assert(nestedResult.text.includes("if(true)"));

  registerConfig(6, { NotAClangFormatOption: true });
  const invalidDiagnostics = parseJsonBytes(readShared(exports.get_config_diagnostics(6)));
  assert(invalidDiagnostics.length > 0);
  assert.strictEqual(invalidDiagnostics[0].propertyName, "clangFormat");

  registerConfig(7, { BasedOnStyle: "file" });
  const fileDiagnostics = parseJsonBytes(readShared(exports.get_config_diagnostics(7)));
  assert(fileDiagnostics.some((diagnostic) => diagnostic.propertyName === "BasedOnStyle"));

  registerConfig(8, { BasedOnStyle: "LLVM" });
  setFilePath("range.cpp");
  const rangeInput = "int a(){return 1;}\nint b(){return 2;}\n";
  writeShared(rangeInput);
  const rangeEnd = rangeInput.indexOf("\n");
  const rangeStatus = exports.format_range(8, 0, rangeEnd);
  assert.strictEqual(rangeStatus, 1);
  assert.strictEqual(readShared(exports.get_formatted_text()), "int a() { return 1; }\nint b(){return 2;}\n");

  setFilePath("range.cpp");
  writeShared("int a(){return 1;}\n");
  const badRangeStatus = exports.format_range(8, 50, 10);
  assert.strictEqual(badRangeStatus, 2);
  assert.strictEqual(readShared(exports.get_error_text()), "invalid format range");

  writeShared(JSON.stringify({ oldVersion: "0.0.1", config: { BasedOnStyle: "LLVM" } }));
  assert.deepStrictEqual(parseJsonBytes(readShared(exports.check_config_updates())), { kind: "ok", data: [] });

  for (let i = 0; i < 100; i++) {
    const result = formatText(1, "repeat.cpp", `int f${i}(){return ${i};}\n`);
    assert.strictEqual(result.status, 1);
    assert(result.text.includes(`int f${i}() { return ${i}; }`));
  }

  console.log("plugin ABI tests passed");
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
