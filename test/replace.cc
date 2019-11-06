#include <vector>
#include <string>
#include <string_view>
#include <cstdio>

std::string ReplaceAll(std::string_view str, std::string_view rep,
                       std::string_view to) {
  std::string s;
  s.reserve(str.size());
  while (!str.empty()) {
    auto pos = str.find(rep);
    if (pos == std::string::npos) {
      s.append(str);
      break;
    }
    s.append(str.substr(0, pos)).append(to);
    str.remove_prefix(pos + rep.size());
  }
  return s;
}

int main() {
  constexpr std::string_view svs[] = {"dewdhedewewdeAB", "dewdewdewdewqdewdB",
                                      "vvvvvvv", "ABABABABABABAB",
                                      "ZZAHHBABBDDBAAB"};
  for (auto s : svs) {
    fprintf(stderr, "%s -> [%s]\n", s.data(), ReplaceAll(s, "AB", "B").data());
  }
  return 0;
}
