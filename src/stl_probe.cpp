#include <string>
#include <vector>

extern "C" __attribute__((visibility("default"))) int stl_probe() {
  std::string text = "int main(){return 1;}";
  std::vector<int> values;
  values.push_back(static_cast<int>(text.size()));
  values.push_back(7);
  return values[0] + values[1];
}
