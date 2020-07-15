///
#include "kmutils.hpp"
#include <bela/match.hpp>

namespace krycekium {
bool FolderIsEmpty(std::wstring_view dir) {
  auto xdir = bela::StringCat(dir, L"\\*.*");
  WIN32_FIND_DATA fdata;
  auto hFind = FindFirstFileW(xdir.data(), &fdata);
  if (hFind == INVALID_HANDLE_VALUE) {
    return true;
  }
  constexpr std::wstring_view dot = L".";
  constexpr std::wstring_view dotdot = L"..";
  for (;;) {
    if (dotdot != fdata.cFileName && dot != fdata.cFileName) {
      FindClose(hFind);
      return false;
    }
    if (FindNextFile(hFind, &fdata) != TRUE) {
      break;
    }
  }
  FindClose(hFind);
  return true;
}

inline bool EnableSameFolder(std::wstring_view *msi) {
  constexpr std::wstring_view suffix[] = {L".msi", L".msp"};
  for (auto s : suffix) {
    if (bela::EndsWithIgnoreCase(*msi, s)) {
      msi->remove_suffix(s.size());
      return true;
    }
  }
  return false;
}

bool IsSuffixEnabled(std::wstring_view msi) {
  // NOTHING
  return EnableSameFolder(&msi);
}

std::optional<std::wstring> SameFolder(std::wstring_view msi) {
  if (!EnableSameFolder(&msi)) {
    return std::nullopt;
  }
  if (FolderIsEmpty(msi)) {
    return std::make_optional<std::wstring>(msi);
  }
  return std::nullopt;
}

} // namespace krycekium