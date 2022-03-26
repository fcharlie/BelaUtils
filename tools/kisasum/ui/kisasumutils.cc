///
#include "ui.hpp"
#include <PathCch.h>
#include <json.hpp>
#include <bela/io.hpp>
#include <charconv>

namespace kisasum::ui {

void resolovecolor(nlohmann::json &j, const char *name, bela::color &value) {
  auto it = j.find(name);
  if (it == j.end()) {
    return;
  }
  auto s = it->get<std::string_view>();
  bela::color defaults;
  defaults.abgr = (std::numeric_limits<uint32_t>::max)();
  if (auto z = bela::color::decode(s, defaults); z != defaults) {
    value = z;
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
      title = bela::encode_into<char,wchar_t>(it->get<std::string_view>());
    }
    if (auto it = j.find("font"); it != j.end()) {
      font = bela::encode_into<char,wchar_t>(it->get<std::string_view>());
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
    cj["panel"] = panelcolor.encode<char>();
    cj["text"] = textcolor.encode<char>();
    cj["content"] = contentcolor.encode<char>();
    cj["label"] = labelcolor.encode<char>();
    j["color"] = cj;
    j["font"] = bela::encode_into<wchar_t, char>(font);
    j["title"] = bela::encode_into<wchar_t, char>(title);
    auto s = j.dump(4);
    if (!bela::io::WriteTextAtomic(s, profile, ec)) {
      return false;
    }
  } catch (const std::exception &e) {
    ec = bela::make_error_code(1, L"flush color: ", bela::encode_into<char,wchar_t>(e.what()));
    return false;
  }
  return true;
}
} // namespace kisasum::ui