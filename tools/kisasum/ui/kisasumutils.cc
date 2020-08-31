///
#include "ui.hpp"
#include <PathCch.h>
#include <json.hpp>
#include <bela/io.hpp>
#include <charconv>

namespace kisasum::ui {

namespace color {
inline bool decode(std::string_view sr, COLORREF &cr) {
  if (sr.empty()) {
    return false;
  }
  if (sr.front() == '#') {
    sr.remove_prefix(1);
  }
  if (sr.size() != 6) {
    return false;
  }
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  auto r1 = std::from_chars(sr.data(), sr.data() + 2, r, 16);
  auto r2 = std::from_chars(sr.data() + 2, sr.data() + 4, g, 16);
  auto r3 = std::from_chars(sr.data() + 4, sr.data() + 6, b, 16);
  if (r1.ec != std::errc{} || r2.ec != std::errc{} || r3.ec != std::errc{}) {
    return false;
  }
  cr = RGB(r, g, b);
  return true;
}

inline std::string encode(COLORREF cr) {
  std::string s;
  s.resize(8);
  uint8_t r = GetRValue(cr);
  uint8_t g = GetGValue(cr);
  uint8_t b = GetBValue(cr);
  _snprintf(s.data(), 8, "#%02x%02x%02x", r, g, b);
  s.resize(7);
  return s;
}
} // namespace color

void resolovecolor(nlohmann::json &j, const char *name, uint32_t &value) {
  auto it = j.find(name);
  if (it == j.end()) {
    return;
  }
  auto s = it->get<std::string_view>();
  COLORREF cr;
  if (color::decode(s, cr)) {
    value = static_cast<uint32_t>(cr);
  }
}

bool WindowSettings::Update() {
  if (profile.empty()) {
    return false;
  }
  bela::error_code ec;
  FILE *fd = nullptr;
  if (_wfopen_s(&fd, profile.data(), L"rb") != 0) {
    return false;
  }
  auto closer = bela::finally([&] { fclose(fd); });
  try {
    /* code */
    auto j = nlohmann::json::parse(fd, nullptr, true, true);
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

bool WindowSettings::Flush(bela::error_code &ec) {
  if (profile.empty()) {
    ec = bela::make_error_code(1, L"profile not found");
    return false;
  }
  try {
    nlohmann::json j;
    nlohmann::json cj;
    cj["panel"] = color::encode(panelcolor);
    cj["text"] = color::encode(textcolor);
    cj["content"] = color::encode(contentcolor);
    cj["label"] = color::encode(labelcolor);
    j["color"] = cj;
    j["font"] = bela::ToNarrow(font);
    j["title"] = bela::ToNarrow(title);
    auto s = j.dump(4);
    if (!bela::io::WriteTextAtomic(s, profile, ec)) {
      return false;
    }
  } catch (const std::exception &e) {
    ec = bela::make_error_code(1, L"flush color: ", bela::ToWide(e.what()));
    return false;
  }
  return true;
}
} // namespace kisasum::ui