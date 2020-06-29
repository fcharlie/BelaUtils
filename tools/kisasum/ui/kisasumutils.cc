///
#include "ui.hpp"
#include <PathCch.h>
#include <json.hpp>
#include <bela/path.hpp>
#include <bela/io.hpp>
#include <charconv>

namespace kisasum::ui {

namespace color {
int32_t Decode(std::string_view color) {
  if (color.empty()) {
    return -1;
  }
  if (color[0] == '#') {
    auto s = color.substr(1);
    uint32_t cl = 0;
    auto result = std::from_chars(s.data(), s.data() + s.size(), cl, 16);
    if (result.ec != std::errc{}) {
      return -1;
    }
    return cl;
  }
  // RGB not support now
  return -1;
}

std::string Encode(uint32_t color) {
  char buffer[256];
  auto result = std::to_chars(buffer, buffer + sizeof(buffer), color, 16);
  if (result.ec != std::errc{}) {
    return "";
  }
  std::string_view sv(buffer, result.ptr - buffer);
  if (sv.size() > 6) {
    return "";
  }
  constexpr std::string_view zero = "#000000";
  std::string s;
  s.append(zero.substr(0, zero.size() - sv.size())).append(sv);
  return s;
}

} // namespace color
constexpr const std::wstring_view profilename = L"kisasum-ui.json";

void resolovecolor(nlohmann::json &j, const char *name, uint32_t &value) {
  auto it = j.find(name);
  if (it == j.end()) {
    return;
  }
  auto s = it->get<std::string>();
  if (auto val = color::Decode(s); val >= 0) {
    value = static_cast<uint32_t>(val);
  }
}

bool WindowSettings::Update() {
  bela::error_code ec;
  auto parent = bela::ExecutableParent(ec);
  if (!parent) {
    return false;
  }
  auto profile = bela::StringCat(*parent, L"\\", profilename);
  FILE *fd = nullptr;
  if (_wfopen_s(&fd, profile.data(), L"rb") != 0) {
    return false;
  }
  auto closer = bela::finally([&] { fclose(fd); });
  try {
    /* code */
    auto j = nlohmann::json::parse(fd);
    if (auto it = j.find("title"); it != j.end()) {
      title = bela::ToWide(it->get<std::string_view>());
    }
    if (auto it = j.find("font"); it != j.end()) {
      font = bela::ToWide(it->get<std::string_view>());
    }
    if (auto it = j.find("color"); it != j.end()) {
      resolovecolor(it.value(), "panel", panelcolor);
      resolovecolor(it.value(), "content", contentcolor);
      resolovecolor(it.value(), "text", textcolor);
      resolovecolor(it.value(), "label", labelcolor);
    }

  } catch (const std::exception &) {
    return false;
  }
  return true;
}

bool WindowSettings::Flush() {
  bela::error_code ec;
  auto parent = bela::ExecutableParent(ec);
  if (!parent) {
    return false;
  }
  auto profile = bela::StringCat(*parent, L"\\", profilename);
  try {
    nlohmann::json j;
    nlohmann::json cj;
    cj["panel"] = color::Encode(panelcolor);
    cj["text"] = color::Encode(textcolor);
    cj["content"] = color::Encode(contentcolor);
    cj["label"] = color::Encode(labelcolor);
    j["color"] = cj;
    j["font"] = bela::ToNarrow(font);
    j["title"] = bela::ToNarrow(title);
    auto s = j.dump(4);
    bela::error_code ec;
    if (!bela::io::WriteTextAtomic(s, profile, ec)) {
      return false;
    }
  } catch (const std::exception &) {
    return false;
  }
  return true;
}
} // namespace kisasum::ui