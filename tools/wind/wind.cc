// A simple program download network resource
#include <bela/parseargv.hpp>
#include <filesystem>
#include "wind.hpp"
#include "net.hpp"
#include "resource.h"

namespace baulk {
bool IsDebugMode = false;
wchar_t UserAgent[UerAgentMaximumLength] = L"Wget/5.0 (Baulk)";
} // namespace baulk

void Usage() {
  constexpr std::wstring_view usage = LR"(wind - Interesting download tool
Usage: wind [option]... [url]...
  -h|--help        Show usage text and quit
  -v|--version     Show version number and quit
  -V|--verbose     Make the operation more talkative
  -f|--force       Turn on force mode. such as overwrite exists file
  -w|--workdir     Save the file to the specified directory
  -o|--out         Write file to the specified path
  --https-proxy    Use this proxy. Equivalent to setting the environment variable 'HTTPS_PROXY'

Example:
  wind https://aka.ms/win32-x64-user-stable

)";
  bela::terminal::WriteAuto(stderr, usage);
}

void Version() {
  bela::FPrintF(stdout, L"wind %s\nRelease:    %s\nCommit:     %s\nBuild Time: %s\n",
                BELAUTILS_VERSION, BELAUTILS_REFNAME, BELAUTILS_REVISION, BELAUTILS_BUILD_TIME);
}

struct Whirlwind {
  std::vector<std::wstring> urls;
  std::wstring workdir;
  std::wstring outfile;
  bool force{false};
};

bool ParseArgv(int argc, wchar_t **argv, Whirlwind &ww) {
  bela::ParseArgv pa(argc, argv);
  pa.Add(L"help", bela::no_argument, L'h')
      .Add(L"version", bela::no_argument, L'v')
      .Add(L"verbose", bela::no_argument, L'V')
      .Add(L"workdir", bela::required_argument, L'w')
      .Add(L"force", bela::no_argument, L'f')
      .Add(L"out", bela::required_argument, L'o')
      .Add(L"user-agent", bela::required_argument, 'A')
      .Add(L"https-proxy", bela::required_argument, 1001); // option
  bela::error_code ec;
  auto ret = pa.Execute(
      [&](int val, const wchar_t *oa, const wchar_t *) {
        switch (val) {
        case 'h':
          Usage();
          exit(0);
        case 'v':
          Version();
          exit(0);
        case 'V':
          baulk::IsDebugMode = true;
          break;
        case 'f':
          ww.force = true;
          break;
        case 'w':
          ww.workdir = oa;
          break;
        case 'o':
          ww.outfile = oa;
          break;
        case 'A':
          if (auto len = wcslen(oa); len < 256) {
            wmemcmp(baulk::UserAgent, oa, len);
            baulk::UserAgent[len] = 0;
          }
          break;
        case 1001:
          SetEnvironmentVariableW(L"HTTPS_PROXY", oa);
          break;
        default:
          break;
        }
        return true;
      },
      ec);
  if (!ret) {
    bela::FPrintF(stderr, L"ParseArgv: %s\n", ec.message);
    return false;
  }
  if (pa.UnresolvedArgs().empty()) {
    bela::FPrintF(stderr, L"wind: missing URL\n");
    Usage();
    return false;
  }
  for (const auto p : pa.UnresolvedArgs()) {
    ww.urls.emplace_back(std::wstring(p));
  }
  if (ww.workdir.empty()) {
    std::error_code e;
    auto cwd = std::filesystem::current_path(e);
    if (e) {
      ec = bela::from_std_error_code(e);
      bela::FPrintF(stderr, L"unable resolve current path: %s\n", ec.message);
      return false;
    }
    ww.workdir = cwd.wstring();
  }
  return true;
}

int wmain(int argc, wchar_t **argv) {
  Whirlwind ww;
  if (!ParseArgv(argc, argv, ww)) {
    return 1;
  }
  bela::error_code ec;
  if (ww.urls.size() == 1 && !ww.outfile.empty()) {
    auto u = ww.urls[0];
    auto file = baulk::net::WinGet(u, ww.workdir, ww.force, ec);
    if (!file) {
      bela::FPrintF(stderr, L"download failed: \x1b[31m%s\x1b[0m\n", ec.message);
      return 1;
    }
    if (MoveFileExW(file->data(), ww.outfile.data(),
                    MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) != TRUE) {
      ec = bela::make_system_error_code();
      bela::FPrintF(stderr, L"unable move %s to %s error: \x1b[31m%s\x1b[0m\n", *file, ww.outfile,
                    ec.message);
      return 1;
    }
    std::error_code e;
    auto p = std::filesystem::absolute(ww.outfile, e);
    if (e) {
      ec = bela::from_std_error_code(e);
      bela::FPrintF(stderr, L"unable resolve absolute path: \x1b[31m%s\x1b[0m\n", ec.message);
      return 1;
    }
    bela::FPrintF(stdout, L"\x1b[32m'%s' saved\x1b[0m\n", p.wstring());
    return 0;
  }
  size_t success = 0;
  for (const auto &u : ww.urls) {
    auto file = baulk::net::WinGet(u, ww.workdir, ww.force, ec);
    if (!file) {
      bela::FPrintF(stderr, L"download failed: \x1b[31m%s\x1b[0m\n", ec.message);
      continue;
    }
    bela::FPrintF(stdout, L"\x1b[32m'%s' saved\x1b[0m\n", *file);
  }
  return success == ww.urls.size() ? 0 : 1;
}