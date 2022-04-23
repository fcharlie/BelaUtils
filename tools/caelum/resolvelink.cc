///
#include "caelum.hpp"
#include <bela/path.hpp>
#include <bela/endian.hpp>
#include <bela/buffer.hpp>
#include <bela/io.hpp>
#include "shl.hpp"

namespace caelum {

class shl_bytes_view {
public:
  shl_bytes_view(bela::bytes_view bv_) : bv(bv_) {}
  // --> perpare shl memview
  bool prepare() {
    if (bv.size() < sizeof(shl::shell_link_t)) {
      return false;
    }
    auto dwSize = bv.cast_fromle<uint32_t>(0);
    if (dwSize != 0x0000004C) {
      return false;
    }
    constexpr uint8_t shuuid[] = {0x1, 0x14, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x46};
    if (!bv.match_bytes_with(4, shuuid)) {
      return false;
    }
    linkflags_ = bv.cast_fromle<uint32_t>(20);
    IsUnicode = (linkflags_ & shl::IsUnicode) != 0;
    return true;
  }

  const char *data() const { return reinterpret_cast<const char *>(bv.data()); }
  size_t size() const { return bv.size(); }

  template <typename T>
  requires bela::standard_layout<T>
  const T *checked_cast(size_t off) const { return bv.checked_cast<T>(off); }

  uint32_t linkflags() const { return linkflags_; }

  bool stringdata(size_t pos, std::wstring &sd, size_t &sdlen) const {
    if (pos + 2 > bv.size()) {
      return false;
    }
    // CountCharacters (2 bytes): A 16-bit, unsigned integer that specifies
    // either the number of characters, defined by the system default code page,
    // or the number of Unicode characters found in the String field. A value of
    // zero specifies an empty string.
    // String (variable): An optional set of characters, defined by the system
    // default code page, or a Unicode string with a length specified by the
    // CountCharacters field. This string MUST NOT be NULL-terminated.

    auto len = bv.cast_fromle<uint16_t>(pos); /// Ch
    if (IsUnicode) {
      sdlen = len * 2 + 2;
      if (sdlen + pos >= bv.size()) {
        return false;
      }
      auto *p = reinterpret_cast<const uint16_t *>(bv.data() + pos + 2);
      sd.clear();
      for (size_t i = 0; i < len; i++) {
        // Winodws UTF16LE
        sd.push_back(bela::fromle(p[i]));
      }
      return true;
    }

    sdlen = len + 2;
    if (sdlen + pos >= bv.size()) {
      return false;
    }
    auto w = bela::fromascii(bv.make_cstring_view(pos + 2, len));
    return true;
  }

  bool stringvalue(size_t pos, bool isu, std::wstring &su) {
    if (pos >= bv.size()) {
      return false;
    }
    if (!isu) {
      su = bela::fromascii(bv.make_cstring_view(pos));
      return true;
    }
    auto it = (const wchar_t *)(bv.data() + pos);
    auto end = it + (bv.size() - pos) / 2;
    for (; it != end; it++) {
      if (*it == 0) {
        return true;
      }
      su.push_back(bela::fromle(*it));
    }
    return false;
  }

private:
  bela::bytes_view bv;
  uint32_t linkflags_;
  bool IsUnicode{false};
};

inline std::wstring FullPath(std::wstring_view sv) {
  //::GetFullPathNameW()
  std::wstring ws(0x8000, L'\0');
  auto N = GetFullPathNameW(sv.data(), 0x8000, ws.data(), nullptr);
  if (N > 0 && N < 0x8000) {
    ws.resize(N);
    return ws;
  }
  return L"";
}

struct link_details_t {
  std::wstring target;
  std::wstring relativepath;
};

std::optional<std::wstring> ResolveShLink(std::wstring_view sv, bela::error_code &ec) {
  if (sv.size() < 4 || sv.compare(sv.size() - 4, 4, L".lnk") != 0) {
    return std::make_optional(std::wstring(sv));
  }
  auto fd = bela::io::NewFile(sv, ec);
  if (!fd) {
    return std::nullopt;
  }
  auto size = fd->Size(ec);
  if (size == bela::SizeUnInitialized) {
    return std::nullopt;
  }
  auto minSize = (std::min)(static_cast<size_t>(64 * 1024), static_cast<size_t>(size));
  bela::Buffer buffer(minSize);
  if (!fd->ReadFull(buffer, minSize, ec)) {
    return std::nullopt;
  }
  shl_bytes_view shm(buffer.as_bytes_view());
  if (!shm.prepare()) {
    return std::make_optional(std::wstring(sv));
  }
  auto flag = shm.linkflags();
  size_t offset = sizeof(shl::shell_link_t);
  if ((flag & shl::HasLinkTargetIDList) != 0) {
    if (shm.size() <= offset + 2) {
      return std::make_optional(std::wstring(sv));
    }
    auto l = bela::cast_fromle<uint16_t>(shm.data() + offset);
    if (l + 2 + offset >= shm.size()) {
      return std::make_optional(std::wstring(sv));
    }
    offset += l + 2;
  }
  // LinkINFO https://msdn.microsoft.com/en-us/library/dd871404.aspx

  if ((flag & shl::HasLinkInfo) != 0) {
    auto li = shm.checked_cast<shl::shl_link_infow_t>(offset);
    if (li == nullptr) {
      return std::make_optional(std::wstring(sv));
    }
    auto liflag = bela::fromle(li->dwFlags);
    if ((liflag & shl::VolumeIDAndLocalBasePath) != 0) {
      bool isunicode;
      size_t pos;
      std::wstring target;
      if (bela::fromle(li->cbHeaderSize) < 0x00000024) {
        isunicode = false;
        pos = offset + bela::fromle(li->cbLocalBasePathOffset);
      } else {
        isunicode = true;
        pos = offset + bela::fromle(li->cbLocalBasePathUnicodeOffset);
      }
      if (!shm.stringvalue(pos, isunicode, target)) {
        return std::make_optional(std::wstring(sv));
      }
      return std::make_optional(target);
    }
    offset += bela::fromle(li->cbSize);
  }

  std::wstring placeholder;
  size_t sdlen = 0;
  if ((flag & shl::HasName) != 0) {
    if (shm.stringdata(offset, placeholder, sdlen)) {
      offset += sdlen;
    }
  }
  placeholder.clear();
  if ((flag & shl::HasRelativePath) != 0) {
    if (!shm.stringdata(offset, placeholder, sdlen)) {
      return std::make_optional(std::wstring(sv));
    }
    offset += sdlen;
  }

  auto target = FullPath(bela::StringCat(sv, L"\\", placeholder));
  if (target.empty()) {
    return std::make_optional(std::wstring(sv));
  }
  return std::make_optional(target);
}

std::optional<std::wstring> ResolveLink(std::wstring_view file, bela::error_code &ec) {
  auto p = bela::RealPathEx(file, ec);
  if (!p) {
    return ResolveShLink(file, ec);
  }
  return ResolveShLink(*p, ec);
}
} // namespace caelum