// A simple program download network resource
#include <bela/parseargv.hpp>
#include <filesystem>
#include <baulk/hash.hpp>
#include <baulk/net/client.hpp>
#include <belautilsversion.h>
#include "wind.hpp"

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
  -k|--insecure    Allow insecure server connections when using SSL
  -n|--no-cache    Download directly without caching
  -w|--workdir     Save the file to the specified directory
  -o|--out         Write file to the specified path
  --https-proxy    Use this proxy. Equivalent to setting the environment variable 'HTTPS_PROXY'

Example:
  wind https://aka.ms/win32-x64-user-stable

)";
  bela::terminal::WriteAuto(stderr, usage);
}

void Version() {
  bela::FPrintF(stdout, L"wind %s\nRelease:    %s\nCommit:     %s\nBuild Time: %s\n", BELAUTILS_VERSION,
                BELAUTILS_REFNAME, BELAUTILS_REVISION, BELAUTILS_BUILD_TIME);
}

struct Whirlwind {
  std::vector<std::wstring> urls;
  std::filesystem::path cwd;
  std::filesystem::path dest;
  bool force{false};
};

bool ParseArgv(int argc, wchar_t **argv, Whirlwind &ww) {
  using namespace baulk::net;
  bela::ParseArgv pa(argc, argv);
  pa.Add(L"help", bela::no_argument, L'h')
      .Add(L"version", bela::no_argument, L'v')
      .Add(L"verbose", bela::no_argument, L'V')
      .Add(L"workdir", bela::required_argument, L'w')
      .Add(L"force", bela::no_argument, L'f')
      .Add(L"insecure", bela::no_argument, L'k')
      .Add(L"no-cache", bela::no_argument, L'n')
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
          HttpClient::DefaultClient().DebugMode() = true;
          break;
        case 'f':
          ww.force = true;
          break;
        case 'k':
          HttpClient::DefaultClient().InsecureMode() = true;
          break;
        case 'd':
          // ww.nocache = true;
          break;
        case 'w':
          ww.cwd = oa;
          break;
        case 'o':
          ww.dest = oa;
          break;
        case 'A':
          HttpClient::DefaultClient().UserAgent() = oa;
          break;
        case 1001:
          HttpClient::DefaultClient().ProxyURL() = oa;
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
    ww.urls.emplace_back(std::wstring{p});
  }
  if (ww.cwd.empty()) {
    std::error_code e;
    ww.cwd = std::filesystem::current_path(e);
    if (e) {
      ec = bela::from_std_error_code(e);
      bela::FPrintF(stderr, L"unable resolve current path: %s\n", ec.message);
      return false;
    }
  }
  HttpClient::DefaultClient().InitializeProxyFromEnv();
  return true;
}

void verify_file(const std::filesystem::path &file) {
  std::wstring sha256sum;
  std::wstring blake3sum;
  bela::error_code ec;
  auto filename = file.filename();
  if (!baulk::hash::HashVerify(file.native(), sha256sum, blake3sum, ec)) {
    bela::FPrintF(stderr, L"unable check %s checksum: %v\n", filename.native(), ec);
    return;
  }
  bela::FPrintF(stderr, L"\x1b[34mSHA256:%s %s\x1b[0m\n", sha256sum, filename.native());
  bela::FPrintF(stderr, L"\x1b[34mBLAKE3:%s %s\x1b[0m\n", blake3sum, filename.native());
}

int wmain(int argc, wchar_t **argv) {
  Whirlwind ww;
  if (!ParseArgv(argc, argv, ww)) {
    return 1;
  }

  bela::error_code ec;
  if (ww.urls.size() == 1 && !ww.dest.empty()) {
    auto u = ww.urls[0];
    auto file = baulk::net::WinGet(u, ww.cwd.native(), L"", ww.force, ec);
    if (!file) {
      bela::FPrintF(stderr, L"download failed: \x1b[31m%v\x1b[0m\n", ec);
      return 1;
    }
    std::error_code e;
    if (std::filesystem::rename(*file, ww.dest, e); e) {
      ec = bela::from_std_error_code(e);
      bela::FPrintF(stderr, L"rename target failed: \x1b[31m%v\x1b[0m\n", ec);
      return 1;
    }
    verify_file(ww.dest);
    bela::FPrintF(stdout, L"\x1b[32m'%s' saved\x1b[0m\n", ww.dest.native());
    return 0;
  }
  size_t success = 0;
  for (const auto &u : ww.urls) {
    auto file = baulk::net::WinGet(u, ww.cwd.native(), L"", ww.force, ec);
    if (!file) {
      bela::FPrintF(stderr, L"download failed: \x1b[31m%s\x1b[0m\n", ec);
      continue;
    }
    verify_file(*file);
    bela::FPrintF(stdout, L"\x1b[32m'%s' saved\x1b[0m\n", file->native());
  }
  return success == ww.urls.size() ? 0 : 1;
}