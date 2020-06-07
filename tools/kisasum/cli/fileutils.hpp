///
#ifndef KISASUM_FILEUTILS_HPP
#define KISASUM_FILEUTILS_HPP
#include <bela/base.hpp>

namespace kisasum {
class FileUtils {
public:
  FileUtils() = default;
  FileUtils(const FileUtils &) = delete;
  FileUtils &operator=(const FileUtils &) = delete;
  ~FileUtils() {
    if (FileHandle != INVALID_HANDLE_VALUE) {
      CloseHandle(FileHandle);
    }
  }
  bool Open(std::wstring_view path, bela::error_code &ec);
  int64_t FileSize() const { return filesize; }
  HANDLE P() const { return FileHandle; }

private:
  HANDLE FileHandle{INVALID_HANDLE_VALUE};
  int64_t filesize{0};
};

inline bool FileUtils::Open(std::wstring_view path, bela::error_code &ec) {
  FileHandle = CreateFileW(path.data(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
  if (FileHandle == INVALID_HANDLE_VALUE) {
    ec = bela::make_system_error_code();
    return false;
  }
  LARGE_INTEGER li;
  if (GetFileSizeEx(FileHandle, &li) != TRUE) {
    ec = bela::make_system_error_code();
    return false;
  }
  filesize = li.QuadPart;
  return true;
}

inline std::wstring BaseName(std::wstring_view sv) {
  if (sv.empty()) {
    return L".";
  }
  auto pos = sv.find_last_not_of(L"\\/");
  if (pos == std::wstring_view::npos) {
    return L"/";
  }
  sv.remove_suffix(sv.size() - pos - 1);
  pos = sv.find_last_of(L"\\/");
  if (pos != std::wstring_view::npos) {
    sv.remove_prefix(pos + 1);
  }
  if (sv.empty()) {
    return L".";
  }
  return std::wstring(sv);
}

} // namespace kisasum

#endif