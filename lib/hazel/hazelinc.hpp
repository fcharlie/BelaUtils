///
#ifndef HAZEL_INTERNAL_HPP
#define HAZEL_INTERNAL_HPP
#include "hazel.hpp"
#include <bela/mapview.hpp>
#include <bela/terminal.hpp>

namespace hazel {
// DbgPrint added newline
template <typename... Args> bela::ssize_t DbgPrint(const wchar_t *fmt, Args... args) {
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

template <typename... Args> bela::ssize_t DbgPrintEx(char32_t prefix, const wchar_t *fmt, Args... args) {
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

namespace internal {

/*
 * Compute the length of an array with constant length.  (Use of this method
 * with a non-array pointer will not compile.)
 *
 * Beware of the implicit trailing '\0' when using this with string constants.
 */
template <typename T, size_t N> constexpr size_t ArrayLength(T (&aArr)[N]) { return N; }

template <typename T, size_t N> constexpr T *ArrayEnd(T (&aArr)[N]) { return aArr + ArrayLength(aArr); }

/**
 * std::equal has subpar ergonomics.
 */

template <typename T, typename U, size_t N> bool ArrayEqual(const T (&a)[N], const U (&b)[N]) {
  return std::equal(a, a + N, b);
}

template <typename T, typename U> bool ArrayEqual(const T *const a, const U *const b, const size_t n) {
  return std::equal(a, a + n, b);
}

using byte_t = unsigned char;
typedef enum hazel_status_e : int {
  None = 0,
  Found, ///
  Break
} status_t;
status_t explore_binobj(bela::MemView mv, particulars_result &pr);
status_t explore_fonts(bela::MemView mv, particulars_result &pr);
status_t explore_zip_family(bela::MemView mv, particulars_result &pr);
status_t explore_docs(bela::MemView mv, particulars_result &pr);
status_t explore_images(bela::MemView mv, particulars_result &pr);
status_t explore_archives(bela::MemView mv, particulars_result &pr);
status_t explore_media(bela::MemView mv, particulars_result &pr);
// EX
status_t explore_git_file(bela::MemView mv, particulars_result &pr);
status_t explore_shlink(bela::MemView mv, particulars_result &pr);
/////////// ---
status_t explore_text(bela::MemView mv, particulars_result &pr);
status_t explore_chardet(bela::MemView mv, particulars_result &pr);
} // namespace internal

} // namespace hazel

#endif