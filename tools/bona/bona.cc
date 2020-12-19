///
#include "bona.hpp"
#include <hazel/fs.hpp>
#include <bela/parseargv.hpp>

namespace bona {
bool IsDebugMode = false;

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
    AppenError(j, file, ec);
    bela::FPrintF(stderr, L"Open file %s error: %s\n", file, ec.message);
    return false;
  }
  hazel::hazel_result hr;
  if (!hazel::LookupFile(fd, hr, ec)) {
    AppenError(j, file, ec);
    bela::FPrintF(stderr, L"Lookup file %s error: %s\n", file, ec.message);
    return false;
  }
  if (j != nullptr) {
    nlohmann::json j2;
    j2.emplace("file", bela::ToNarrow(file));
    j2.emplace("description", bela::ToNarrow(hr.description()));
    j2.emplace("size", hr.size());
    if (areRsp) {
      for (const auto &[k, v] : frp.attributes) {
        j2.emplace(bela::ToNarrow(bela::AsciiStrToLower(k)), bela::ToNarrow(v));
      }
    }
    auto append = bela::finally([&]() { j->push_back(std::move(j2)); });
    if (hr.LooksLikeZIP()) {
      return ViewZIP(fd, hr.align_length(), &j2);
    }
    if (hr.LooksLikePE()) {
      return ViewPE(fd, hr.align_length(), &j2);
    }
    if (hr.LooksLikeELF()) {
      return ViewELF(fd, hr.align_length(), &j2);
    }
    if (hr.LooksLikeMachO()) {
      return ViewMachO(fd, hr.align_length(), &j2);
    }
    for (const auto &[k, v] : hr.values()) {
      auto nk = bela::ToNarrow(bela::AsciiStrToLower(k));
      std::visit(hazel::overloaded{
                     [](auto arg) {}, // ignore
                     [&](const std::wstring &sv) { j2.emplace(nk, bela::ToNarrow(sv)); },
                     [&](const std::string &sv) { j2.emplace(nk, sv); },
                     [&](int16_t i) { j2.emplace(nk, i); },
                     [&](int32_t i) { j2.emplace(nk, i); },
                     [&](int64_t i) { j2.emplace(nk, i); },
                     [&](uint16_t i) { j2.emplace(nk, i); },
                     [&](uint32_t i) { j2.emplace(nk, i); },
                     [&](uint64_t i) { j2.emplace(nk, i); },
                     [&](bela::Time t) { j2.emplace(nk, bela::ToNarrow(bela::FormatTime(t))); },
                     [&](const std::vector<std::wstring> &v) {
                       auto av = nlohmann::json::array();
                       for (const auto s : v) {
                         av.emplace_back(bela::ToNarrow(s));
                       }
                       j2.emplace(nk, std::move(av));
                     },
                     [&](const std::vector<std::string> &v) { j2.emplace(nk, v); },
                 },
                 v);
    }
    return true;
  }
  std::wstring space;
  auto alen = hr.align_length();
  if (areRsp) {
    for (const auto &[k, v] : frp.attributes) {
      alen = (std::max)(alen, k.size());
    }
  }
  space.resize(alen + 2, ' ');
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
    return ViewZIP(fd, alen, nullptr);
  }
  if (hr.LooksLikePE()) {
    return ViewPE(fd, alen, nullptr);
  }
  if (hr.LooksLikeELF()) {
    return ViewELF(fd, alen, nullptr);
  }
  if (hr.LooksLikeMachO()) {
    return ViewMachO(fd, alen, nullptr);
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

} // namespace bona

struct options {
  std::vector<std::wstring_view> files;
  bool toJSON{false};
};

bool ParseArgv(int argc, wchar_t **argv, options &opt) {
  bela::ParseArgv pa(argc, argv);
  pa.Add(L"help", bela::no_argument, 'h')
      .Add(L"version", bela::no_argument, 'v')
      .Add(L"verbose", bela::no_argument, 'V')
      .Add(L"json", bela::no_argument, 'J')
      .Add(L"full", bela::no_argument, 'F'); // Detailed mode
  bela::error_code ec;
  auto result = pa.Execute(
      [&](int val, const wchar_t *oa, const wchar_t *raw) {
        switch (val) {
        case 'h':
          break;
        case 'v':
          break;
        case 'V':
          bona::IsDebugMode = true;
          break;
        case 'J':
          opt.toJSON = true;
          break;
        default:
          break;
        }

        return true;
      },
      ec);
  if (!result) {
    bela::FPrintF(stderr, L"ParseArgv: \x1b[31m%s\x1b[0m\n", ec.message);
    return false;
  }
  opt.files = pa.UnresolvedArgs();
  if (opt.files.empty()) {
    bela::FPrintF(stderr, L"bv: missing input file\n");
    return false;
  }
  return true;
}

int wmain(int argc, wchar_t **argv) {
  //
  for (int i = 1; i < argc; i++) {
    bona::ViewFile(argv[i], nullptr);
  }
  return 0;
}