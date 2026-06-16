#include "clang/Format/Format.h"
#include "clang/Tooling/Core/Replacement.h"

#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct RegisteredConfig {
  uint32_t id;
  std::string json;
};

std::vector<uint8_t>& shared_bytes() {
  static auto* value = new std::vector<uint8_t>();
  return *value;
}

std::vector<RegisteredConfig>& registered_configs() {
  static auto* value = new std::vector<RegisteredConfig>();
  return *value;
}

std::string& file_path() {
  static auto* value = new std::string();
  return *value;
}

std::string& override_config_json() {
  static auto* value = new std::string();
  return *value;
}

std::string& formatted_text() {
  static auto* value = new std::string();
  return *value;
}

std::string& error_text() {
  static auto* value = new std::string();
  return *value;
}

std::string take_shared_string() {
  auto& bytes = shared_bytes();
  std::string result(bytes.begin(), bytes.end());
  bytes.clear();
  return result;
}

uint32_t set_shared_string(std::string value) {
  auto& bytes = shared_bytes();
  bytes.assign(value.begin(), value.end());
  return static_cast<uint32_t>(bytes.size());
}

RegisteredConfig* find_config(uint32_t config_id) {
  for (auto& config : registered_configs()) {
    if (config.id == config_id) {
      return &config;
    }
  }
  return nullptr;
}

void upsert_config(uint32_t config_id, std::string json) {
  if (auto* config = find_config(config_id)) {
    config->json = std::move(json);
    return;
  }
  registered_configs().push_back({config_id, std::move(json)});
}

bool json_mentions(std::string_view json, std::string_view key, std::string_view value) {
  auto key_pos = json.find(key);
  if (key_pos == std::string_view::npos) {
    return false;
  }
  return json.find(value, key_pos) != std::string_view::npos;
}

clang::format::FormatStyle resolve_style(uint32_t config_id) {
  std::string_view config_json;
  if (!override_config_json().empty()) {
    config_json = override_config_json();
  } else if (auto* config = find_config(config_id)) {
    config_json = config->json;
  }

  if (json_mentions(config_json, "style", "Google")) {
    return clang::format::getGoogleStyle(clang::format::FormatStyle::LK_Cpp);
  }
  if (json_mentions(config_json, "style", "WebKit")) {
    return clang::format::getWebKitStyle();
  }
  if (json_mentions(config_json, "style", "Microsoft")) {
    return clang::format::getMicrosoftStyle(clang::format::FormatStyle::LK_Cpp);
  }
  return clang::format::getLLVMStyle(clang::format::FormatStyle::LK_Cpp);
}

std::string resolved_style_name(uint32_t config_id) {
  if (auto* config = find_config(config_id)) {
    if (json_mentions(config->json, "style", "Google")) {
      return "Google";
    }
    if (json_mentions(config->json, "style", "WebKit")) {
      return "WebKit";
    }
    if (json_mentions(config->json, "style", "Microsoft")) {
      return "Microsoft";
    }
  }
  return "LLVM";
}

void normalize_file_path() {
  for (auto& ch : file_path()) {
    if (ch == '\\') {
      ch = '/';
    }
  }
}

uint32_t format_inner(uint32_t config_id, uint32_t range_start, uint32_t range_end, bool has_range) {
  std::string input = take_shared_string();
  formatted_text().clear();
  error_text().clear();

  if (!has_range) {
    range_start = 0;
    range_end = static_cast<uint32_t>(input.size());
  }

  if (range_start > range_end || range_end > input.size()) {
    override_config_json().clear();
    error_text() = "invalid format range";
    return 2;
  }

  auto style = resolve_style(config_id);
  override_config_json().clear();

  auto replacements =
      clang::format::reformat(style, input, {clang::tooling::Range(range_start, range_end - range_start)});
  auto result = clang::tooling::applyAllReplacements(input, replacements);
  if (!result) {
    error_text() = "clang-format failed to apply replacements";
    return 2;
  }
  if (*result == input) {
    return 0;
  }
  formatted_text() = std::move(*result);
  return 1;
}

} // namespace

extern "C" {

uint32_t dprint_plugin_version_4() {
  return 4;
}

const uint8_t* get_shared_bytes_ptr() {
  auto& bytes = shared_bytes();
  return bytes.empty() ? nullptr : bytes.data();
}

uint8_t* clear_shared_bytes(uint32_t size) {
  auto& bytes = shared_bytes();
  bytes.assign(size, 0);
  return bytes.empty() ? nullptr : bytes.data();
}

uint32_t get_plugin_info() {
  return set_shared_string(
      "{\"name\":\"dprint-plugin-clang-format-poc\","
      "\"version\":\"0.1.0\","
      "\"configKey\":\"clangFormat\","
      "\"fileExtensions\":[\"c\",\"cc\",\"cpp\",\"cxx\",\"h\",\"hh\",\"hpp\",\"hxx\",\"m\",\"mm\"],"
      "\"helpUrl\":\"\","
      "\"configSchemaUrl\":\"\"}");
}

uint32_t get_license_text() {
  return set_shared_string("Proof-of-concept clang-format plugin using LLVM/Clang LibFormat.");
}

void register_config(uint32_t config_id) {
  upsert_config(config_id, take_shared_string());
}

void release_config(uint32_t config_id) {
  auto& configs = registered_configs();
  for (auto it = configs.begin(); it != configs.end(); ++it) {
    if (it->id == config_id) {
      configs.erase(it);
      return;
    }
  }
}

uint32_t get_config_diagnostics(uint32_t) {
  return set_shared_string("[]");
}

uint32_t get_resolved_config(uint32_t config_id) {
  return set_shared_string("{\"style\":\"" + resolved_style_name(config_id) + "\"}");
}

uint32_t get_config_file_matching(uint32_t) {
  return set_shared_string(
      "{\"fileExtensions\":[\"c\",\"cc\",\"cpp\",\"cxx\",\"h\",\"hh\",\"hpp\",\"hxx\",\"m\",\"mm\"],"
      "\"fileNames\":[]}");
}

void set_file_path() {
  file_path() = take_shared_string();
  normalize_file_path();
}

void set_override_config() {
  override_config_json() = take_shared_string();
}

uint32_t format(uint32_t config_id) {
  return format_inner(config_id, 0, 0, false);
}

uint32_t format_range(uint32_t config_id, uint32_t range_start, uint32_t range_end) {
  return format_inner(config_id, range_start, range_end, true);
}

uint32_t get_formatted_text() {
  auto& text = formatted_text();
  return set_shared_string(std::move(text));
}

uint32_t get_error_text() {
  auto& text = error_text();
  return set_shared_string(std::move(text));
}

uint32_t check_config_updates() {
  return set_shared_string("{\"kind\":\"ok\",\"data\":[]}");
}

} // extern "C"
