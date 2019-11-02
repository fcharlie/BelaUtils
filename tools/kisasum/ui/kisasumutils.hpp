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

} // namespace kisasum

#endif