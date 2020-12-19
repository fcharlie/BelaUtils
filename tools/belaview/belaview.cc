///
#include "belaview.hpp"
#include <hazel/fs.hpp>

namespace belaview {
bool ViewFile(std::wstring_view file, nlohmann::json *j) {
  hazel::fs::FileReparsePoint frp;
  bela::error_code ec;
  std::wstring realPath(file);
  bool areRsp = false;
  if (areRsp = hazel::fs::LookupReparsePoint(file, frp, ec); areRsp) {
    if (auto it = frp.attributes.find(L"Target"); it != frp.attributes.end()) {
      realPath = it->second;
    }
  }
  bela::File fd;
  if (!fd.Open(realPath, ec)) {
    RecordErrorCode(j, file, ec);
    bela::FPrintF(stderr, L"Open file %s error: %s\n", file, ec.message);
    return false;
  }
  hazel::hazel_result hr;
  if (!hazel::LookupFile(fd, hr, ec)) {
    RecordErrorCode(j, file, ec);
    bela::FPrintF(stderr, L"Lookup file %s error: %s\n", file, ec.message);
    return false;
  }
  if (j != nullptr) {
    nlohmann::json j2;
    j2["Description"] = bela::ToNarrow(hr.description());
    j2["Size"] = hr.size();
    if (areRsp) {
      for (const auto &[k, v] : frp.attributes) {
        j2[bela::ToNarrow(k)] = bela::ToNarrow(v);
      }
    }
    if (hr.LooksLikeZIP()) {
      return ViewZIP(fd, &j2);
    }
    if (hr.LooksLikePE()) {
      return ViewPE(fd, &j2);
    }
    if (hr.LooksLikeELF()) {
      return ViewELF(fd, &j2);
    }
    if (hr.LooksLikeMachO()) {
      return ViewMachO(fd, &j2);
    }
    for (const auto &[k, v] : hr.values()) {
      std::visit(hazel::overloaded{
                     [](auto arg) {}, // ignore
                     [&](const std::wstring &sv) { j2[bela::ToNarrow(k)] = bela::ToNarrow(sv); },
                     [&](const std::string &sv) { j2[bela::ToNarrow(k)] = sv; },
                     [&](int16_t i) { j2[bela::ToNarrow(k)] = i; },
                     [&](int32_t i) { j2[bela::ToNarrow(k)] = i; },
                     [&](int64_t i) { j2[bela::ToNarrow(k)] = i; },
                     [&](uint16_t i) { j2[bela::ToNarrow(k)] = i; },
                     [&](uint32_t i) { j2[bela::ToNarrow(k)] = i; },
                     [&](uint64_t i) { j2[bela::ToNarrow(k)] = i; },
                     [&](bela::Time t) { j2[bela::ToNarrow(k)] = bela::ToNarrow(bela::FormatTime(t)); },
                     [&](const std::vector<std::wstring> &v) {
                       auto av = nlohmann::json::array();
                       for (const auto s : v) {
                         av.emplace_back(bela::ToNarrow(s));
                       }
                       j2[bela::ToNarrow(k)] = std::move(av);
                     },
                     [&](const std::vector<std::string> &v) { j2[bela::ToNarrow(k)] = nlohmann::json::array({v}); },
                 },
                 v);
    }
    j->push_back(std::move(j2));
    return true;
  }
  std::wstring space;
  auto len = hr.align_length();
  if (areRsp) {
    for (const auto &[k, v] : frp.attributes) {
      len = (std::max)(len, k.size());
    }
  }
  space.resize(len + 2, ' ');
  std::wstring_view spaceview(space);
  constexpr const std::wstring_view desc = L"Description";
  bela::FPrintF(stdout, L"Description:%s%s\n", spaceview.substr(0, spaceview.size() - 11 - 1), hr.description());
  bela::FPrintF(stdout, L"Size:%s%d\n", spaceview.substr(0, spaceview.size() - 4 - 1), hr.size());
  if (areRsp) {
    for (const auto &[k, v] : frp.attributes) {
      bela::FPrintF(stdout, L"%s:%s%s\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), v);
    }
  }
  if (hr.LooksLikeZIP()) {
    return ViewZIP(fd, nullptr);
  }
  if (hr.LooksLikePE()) {
    return ViewPE(fd, nullptr);
  }
  if (hr.LooksLikeELF()) {
    return ViewELF(fd, nullptr);
  }
  if (hr.LooksLikeMachO()) {
    return ViewMachO(fd, nullptr);
  }
  // https://en.cppreference.com/w/cpp/utility/variant/visit
  for (const auto &[k, v] : hr.values()) {
    bela::FPrintF(stderr, L"%v:%s", k, spaceview.substr(0, spaceview.size() - k.size() - 1));
    std::visit(hazel::overloaded{
                   [](auto arg) {}, // ignore
                   [](const std::wstring &sv) { bela::FPrintF(stdout, L"%s\n", sv); },
                   [](const std::string &sv) { bela::FPrintF(stdout, L"%s\n", sv); },
                   [](int16_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](int32_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](int64_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](uint16_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](uint32_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](uint64_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](bela::Time t) { bela::FPrintF(stdout, L"%s\n", bela::FormatTime(t)); },
                   [spaceview](const std::vector<std::wstring> &v) {
                     if (v.empty()) {
                       bela::FPrintF(stdout, L"\n");
                       return;
                     }
                     bela::FPrintF(stdout, L"%s\n", v[0]);
                     for (size_t i = 1; i < v.size(); i++) {
                       bela::FPrintF(stdout, L"%s%s\n", spaceview, v[0]);
                     }
                   },
                   [spaceview](const std::vector<std::string> &v) {
                     if (v.empty()) {
                       bela::FPrintF(stdout, L"\n");
                       return;
                     }
                     bela::FPrintF(stdout, L"%s\n", v[0]);
                     for (size_t i = 1; i < v.size(); i++) {
                       bela::FPrintF(stdout, L"%s%s\n", spaceview, v[0]);
                     }
                   },
               },
               v);
  }
  return true;
}
} // namespace belaview

int wmain(int argc, wchar_t **argv) {
  //
  for (int i = 1; i < argc; i++) {
    belaview::ViewFile(argv[i], nullptr);
  }
  return 0;
}