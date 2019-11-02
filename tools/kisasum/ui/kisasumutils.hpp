////
#ifndef KISASUMUTILS_HPP
#define KISASUMUTILS_HPP
#include <string_view>
#include <cstring>
#include <string>

namespace kisasum {
inline bool StartsWith(std::string_view text, std::string_view prefix) {
  return prefix.empty() ||
         (text.size() >= prefix.size() &&
          memcmp(text.data(), prefix.data(), prefix.size()) == 0);
}
inline bool EndsWith(std::string_view text, std::string_view suffix) {
  return suffix.empty() || (text.size() >= suffix.size() &&
                            memcmp(text.data() + (text.size() - suffix.size()),
                                   suffix.data(), suffix.size()) == 0);
}
inline bool ConsumePrefix(std::string_view *str, std::string_view expected) {
  if (!StartsWith(*str, expected)) {
    return false;
  }
  str->remove_prefix(expected.size());
  return true;
}
inline bool ConsumeSuffix(std::string_view *str, std::string_view expected) {
  if (!EndsWith(*str, expected)) {
    return false;
  }
  str->remove_suffix(expected.size());
  return true;
}

// ui options
struct KisasumOptions {
  std::wstring title{L"Kismet Immersive"};
  std::wstring font{L"Segoe UI"};
  std::uint32_t panelcolor{0x00BFFF};
  std::uint32_t contentcolor{0xffffff};
  std::uint32_t textcolor{0x000000};
  std::uint32_t labelcolor{0x000000};
};

bool InitializeKisasumOptions(KisasumOptions &options);
bool FlushKisasumOptions(const KisasumOptions &options);
} // namespace kisasum

#endif