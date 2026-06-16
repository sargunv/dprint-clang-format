#include "clang/Format/Format.h"
#include "clang/Tooling/Core/Replacement.h"

#include <string>

extern "C" int clang_format_probe() {
  std::string code = "int main(){return 1;}\n";
  clang::format::FormatStyle style = clang::format::getLLVMStyle();
  auto replacements = clang::format::reformat(
      style, code, {clang::tooling::Range(0, static_cast<unsigned>(code.size()))});
  auto result = clang::tooling::applyAllReplacements(code, replacements);
  if (!result) {
    return -1;
  }
  return static_cast<int>(result->size());
}
