///
#include "caelum.hpp"
#include <bela/path.hpp>
#include <bela/repasepoint.hpp>
#include <bela/endian.hpp>
#include <bela/mapview.hpp>
#include <bela/strcat.hpp>
#include "shl.hpp"

namespace caelum {
std::wstring fromascii(std::string_view sv) {
  auto sz = MultiByteToWideChar(CP_ACP, 0, sv.data(), (int)sv.size(), nullptr, 0);
  std::wstring output;
  output.resize(sz);
  // C++17 must output.data()
  MultiByteToWideChar(CP_ACP, 0, sv.data(), (int)sv.size(), output.data(), sz);
  return output;
}

class shl_memview {
public:
  shl_memview(const char *data__, size_t size__) : data_(data__), size_(size__) {
    //
  }
  // --> perpare shl memview
  bool prepare() {
    constexpr uint8_t shuuid[] = {0x1,  0x14, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0,
                                  0xc0, 0x0,  0x0, 0x0, 0x0, 0x0, 0x0, 0x46};
    if (size_ < sizeof(shl::shell_link_t)) {
      return false;
    }
    auto dwSize = bela::readle<uint32_t>(data_);
    if (dwSize != 0x0000004C) {
      return false;
    }
    if (memcmp(data_ + 4, shuuid, std::size(shuuid)) != 0) {
      return false;
    }
    linkflags_ = bela::readle<uint32_t>(data_ + 20);
    IsUnicode = (linkflags_ & shl::IsUnicode) != 0;
    return true;
  }

  const char *data() const { return data_; }
  size_t size() const { return size_; }

  template <typename T> const T *cast(size_t off) const {
    if (off + sizeof(T) >= size_) {
      return nullptr;
    }
    return reinterpret_cast<const T *>(data_ + off);
  }

  uint32_t linkflags() const { return linkflags_; }

  bool stringdata(size_t pos, std::wstring &sd, size_t &sdlen) const {
    if (pos + 2 > size_) {
      return false;
    }
    // CountCharacters (2 bytes): A 16-bit, unsigned integer that specifies
    // either the number of characters, defined by the system default code page,
    // or the number of Unicode characters found in the String field. A value of
    // zero specifies an empty string.
    // String (variable): An optional set of characters, defined by the system
    // default code page, or a Unicode string with a length specified by the
    // CountCharacters field. This string MUST NOT be NULL-terminated.

    auto len = bela::readle<uint16_t>(data_ + pos); /// Ch
    if (IsUnicode) {
      sdlen = len * 2 + 2;
      if (sdlen + pos >= size_) {
        return false;
      }
      auto *p = reinterpret_cast<const uint16_t *>(data_ + pos + 2);
      sd.clear();
      for (size_t i = 0; i < len; i++) {
        // Winodws UTF16LE
        sd.push_back(bela::swaple(p[i]));
      }
      return true;
    }

    sdlen = len + 2;
    if (sdlen + pos >= size_) {
      return false;
    }
    auto w = fromascii(std::string_view(data_ + pos + 2, len));
    return true;
  }

  bool stringvalue(size_t pos, bool isu, std::wstring &su) {
    if (pos >= size_) {
      return false;
    }
    if (!isu) {
      auto begin = data_ + pos;
      auto end = data_ + size_;
      for (auto it = begin; it != end; it++) {
        if (*it == 0) {
          su = fromascii(std::string_view(begin, it - begin));
          return true;
        }
      }
      return false;
    }
    auto it = (const wchar_t *)(data_ + pos);
    auto end = it + (size_ - pos) / 2;
    for (; it != end; it++) {
      if (*it == 0) {
        return true;
      }
      su.push_back(bela::swaple(*it));
    }
    return false;
  }

private:
  const char *data_{nullptr};
  size_t size_{0};
  uint32_t linkflags_;
  bool IsUnicode{false};
};

inline std::wstring PathAbsolute(std::wstring_view sv) {
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
  bela::MapView mv;
  if (!mv.MappingView(sv, ec, sizeof(shl::shell_link_t), 64 * 1024)) {
    return std::nullopt;
  }
  auto mmv = mv.subview();
  shl_memview shm(reinterpret_cast<const char *>(mmv.data()), mmv.size());
  if (!shm.prepare()) {
    return std::make_optional(std::wstring(sv));
  }
  auto flag = shm.linkflags();
  size_t offset = sizeof(shl::shell_link_t);
  if ((flag & shl::HasLinkTargetIDList) != 0) {
    if (shm.size() <= offset + 2) {
      return std::make_optional(std::wstring(sv));
    }
    auto l = bela::readle<uint16_t>(shm.data() + offset);
    if (l + 2 + offset >= shm.size()) {
      return std::make_optional(std::wstring(sv));
    }
    offset += l + 2;
  }
  // LinkINFO https://msdn.microsoft.com/en-us/library/dd871404.aspx

  if ((flag & shl::HasLinkInfo) != 0) {
    auto li = shm.cast<shl::shl_link_infow_t>(offset);
    if (li == nullptr) {
      return std::make_optional(std::wstring(sv));
    }
    auto liflag = bela::swaple(li->dwFlags);
    if ((liflag & shl::VolumeIDAndLocalBasePath) != 0) {
      bool isunicode;
      size_t pos;
      std::wstring target;
      if (bela::swaple(li->cbHeaderSize) < 0x00000024) {
        isunicode = false;
        pos = offset + bela::swaple(li->cbLocalBasePathOffset);
      } else {
        isunicode = true;
        pos = offset + bela::swaple(li->cbLocalBasePathUnicodeOffset);
      }
      if (!shm.stringvalue(pos, isunicode, target)) {
        return std::make_optional(std::wstring(sv));
      }
      return std::make_optional(target);
    }
    offset += bela::swaple(li->cbSize);
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

  auto target = PathAbsolute(bela::StringCat(sv, L"\\", placeholder));
  if (target.empty()) {
    return std::make_optional(std::wstring(sv));
  }
  return std::make_optional(target);
}

std::optional<std::wstring> FindAttributeName(const std::vector<bela::FileAttributePair> &attrs,
                                              std::wstring_view name) {
  for (const auto &a : attrs) {
    if (name == a.name) {
      return std::make_optional(a.value);
    }
  }
  return std::nullopt;
}

inline bool IsTarget(unsigned long i) {
  return i == bela::ReparsePointTagIndex::APPEXECLINK || i == bela::ReparsePointTagIndex::SYMLINK ||
         i == bela::ReparsePointTagIndex::MOUNT_POINT;
}

std::optional<std::wstring> ResolveLink(std::wstring_view file, bela::error_code &ec) {
  bela::ReparsePoint rp;
  if (!rp.Analyze(file, ec)) {
    if (ec.code != ERROR_NOT_A_REPARSE_POINT) {
      return std::nullopt;
    }
    return ResolveShLink(file, ec);
  }
  if (IsTarget(rp.ReparseTagValue())) {
    return FindAttributeName(rp.Attributes(), L"Target");
  }
  return std::make_optional(std::wstring(file));
}
} // namespace caelum