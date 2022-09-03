//
#ifndef PLACEMENT_HPP
#define PLACEMENT_HPP
#include <bela/io.hpp>
#include <bela/path.hpp>
#include <filesystem>
#include "json.hpp"

namespace belautils {
inline bool LoadPlacement(std::wstring_view file, WINDOWPLACEMENT &placement) {
  FILE *fd = nullptr;
  if (auto en = _wfopen_s(&fd, file.data(), L"rb"); en != 0) {
    return false;
  }
  auto closer = bela::finally([&] { fclose(fd); });
  try {
    auto j = nlohmann::json::parse(fd, nullptr, true, true);
    placement.flags = j["flags"];
    placement.ptMaxPosition.x = j["ptMaxPosition.X"];
    placement.ptMaxPosition.y = j["ptMaxPosition.Y"];
    placement.ptMinPosition.x = j["ptMinPosition.X"];
    placement.ptMinPosition.y = j["ptMinPosition.Y"];
    placement.showCmd = j["showCmd"];
    placement.rcNormalPosition.bottom = j["rcNormalPosition.bottom"];
    placement.rcNormalPosition.left = j["rcNormalPosition.left"];
    placement.rcNormalPosition.right = j["rcNormalPosition.right"];
    placement.rcNormalPosition.top = j["rcNormalPosition.top"];
  } catch (const std::exception &) {
    return false;
  }
  return true;
}
inline void SavePlacement(std::wstring_view file, const WINDOWPLACEMENT &placement) {
  std::filesystem::path p(file);
  std::error_code e;
  if (std::filesystem::create_directories(p.parent_path(), e); e) {
    return;
  }
  nlohmann::json j;
  j["flags"] = placement.flags;
  j["ptMaxPosition.X"] = placement.ptMaxPosition.x;
  j["ptMaxPosition.Y"] = placement.ptMaxPosition.y;
  j["ptMinPosition.X"] = placement.ptMinPosition.x;
  j["ptMinPosition.Y"] = placement.ptMinPosition.y;
  j["showCmd"] = placement.showCmd;
  j["rcNormalPosition.bottom"] = placement.rcNormalPosition.bottom;
  j["rcNormalPosition.left"] = placement.rcNormalPosition.left;
  j["rcNormalPosition.right"] = placement.rcNormalPosition.right;
  j["rcNormalPosition.top"] = placement.rcNormalPosition.top;
  bela::error_code ec;
  bela::io::AtomicWriteText(file, bela::io::as_bytes<char>(j.dump(4)), ec);
}
} // namespace belautils

#endif