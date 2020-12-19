///
#ifndef bona_HPP
#define bona_HPP
#include <bela/terminal.hpp>
#include <bela/pe.hpp>
#include <bela/time.hpp>
#include <bela/datetime.hpp>
#include <hazel/hazel.hpp>
#include <hazel/elf.hpp>
#include <hazel/macho.hpp>
#include <json.hpp>

namespace bona {
extern bool IsDebugMode;
// DbgPrint added newline
template <typename... Args> bela::ssize_t DbgPrint(const wchar_t *fmt, const Args &...args) {
  if (!IsDebugMode) {
    return 0;
  }
  const bela::format_internal::FormatArg arg_array[] = {args...};
  std::wstring str;
  str.append(L"\x1b[33m* ");
  bela::format_internal::StrAppendFormatInternal(&str, fmt, arg_array, sizeof...(args));
  if (str.back() == '\n') {
    str.pop_back();
  }
  str.append(L"\x1b[0m\n");
  return bela::terminal::WriteAuto(stderr, str);
}
inline bela::ssize_t DbgPrint(const wchar_t *fmt) {
  if (!IsDebugMode) {
    return 0;
  }
  std::wstring_view msg(fmt);
  if (!msg.empty() && msg.back() == '\n') {
    msg.remove_suffix(1);
  }
  return bela::terminal::WriteAuto(stderr, bela::StringCat(L"\x1b[33m* ", msg, L"\x1b[0m\n"));
}

template <typename... Args> bela::ssize_t DbgPrintEx(char32_t prefix, const wchar_t *fmt, const Args &...args) {
  if (!IsDebugMode) {
    return 0;
  }
  const bela::format_internal::FormatArg arg_array[] = {args...};
  auto str = bela::StringCat(L"\x1b[32m* ", prefix, L" ");
  bela::format_internal::StrAppendFormatInternal(&str, fmt, arg_array, sizeof...(args));
  if (str.back() == '\n') {
    str.pop_back();
  }
  str.append(L"\x1b[0m\n");
  return bela::terminal::WriteAuto(stderr, str);
}
inline bela::ssize_t DbgPrintEx(char32_t prefix, const wchar_t *fmt) {
  if (!IsDebugMode) {
    return 0;
  }
  std::wstring_view msg(fmt);
  if (!msg.empty() && msg.back() == '\n') {
    msg.remove_suffix(1);
  }
  return bela::terminal::WriteAuto(stderr, bela::StringCat(L"\x1b[32m", prefix, L" ", msg, L"\x1b[0m\n"));
}
inline void AppenError(nlohmann::json *j, std::wstring_view file, const bela::error_code &ec) {
  if (j = nullptr) {
    return;
  }
  try {
    nlohmann::json j2;
    j2.emplace("code", ec.code);
    j2.emplace("file", bela::ToNarrow(file));
    j2.emplace("message", bela::ToNarrow(ec.message));
    j->push_back(std::move(j2));
  } catch (const std::exception &) {
  }
}

inline void AssignError(nlohmann::json *j, const bela::error_code &ec) {
  if (j = nullptr) {
    return;
  }
  try {
    j->emplace("code", ec.code);
    j->emplace("message", bela::ToNarrow(ec.message));
  } catch (const std::exception &) {
  }
}

bool ViewPE(bela::File &fd, size_t alen, nlohmann::json *j);
bool ViewZIP(bela::File &fd, size_t alen, nlohmann::json *j);
bool ViewELF(bela::File &fd, size_t alen, nlohmann::json *j);
bool ViewMachO(bela::File &fd, size_t alen, nlohmann::json *j);

} // namespace bona

#endif