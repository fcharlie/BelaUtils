///
#include <bela/parseargv.hpp>
#include <bela/terminal.hpp>
#include <bela/match.hpp>
#include <bela/ascii.hpp>
#include <bela/codecvt.hpp>
#include <bela/path.hpp>
#include <vector>
#include <optional>
#include <json.hpp>
#include "../../../lib/hashlib/sumizer.hpp"
#include "fileutils.hpp"
#include "indicators.hpp"

void usage() {
  const std::wstring_view ua = LR"(OVERVIEW: kisasum 1.0
USAGE: kisasum [options] <input>
OPTIONS:
  -a, --algorithm  Hash Algorithm,support algorithm described below.
                   Algorithm Ignore case, default sha256
  -f, --format     Return information about hash in a format described below.
  -h, --help       Print usage and exit.
  -v, --version    Print version and exit.

Algorithm:
  SHA224     SHA256     SHA384     SHA512
  SHA3-224   SHA3-256   SHA3-384   SHA3-512
  BLAKE3     BLAKE2s    BLAKE2b

Formats:
  text     format to text, support progress
  json     format to json
)";
  bela::FPrintF(stderr, L"%s\n", ua);
}

struct kisasum_options {
  std::wstring_view alg{L"SHA256"};
  std::wstring_view format;
  std::vector<std::wstring_view> files;
};

struct kisasum_result {
  std::wstring filename;
  std::wstring hashhex;
};

bool parse_options(int argc, wchar_t **argv, kisasum_options &opt) {
  bela::ParseArgv pa(argc, argv);
  pa.Add(L"algorithm", bela::required_argument, 'a')
      .Add(L"format", bela::required_argument, 'f')
      .Add(L"json", bela::no_argument, 'j')
      .Add(L"help", bela::no_argument, 'h')
      .Add(L"version", bela::no_argument, 'v');
  bela::error_code ec;
  auto ret = pa.Execute(
      [&](int val, const wchar_t *oa, const wchar_t *) {
        switch (val) {
        case 'a':
          opt.alg = oa;
          break;
        case 'f':
          opt.format = oa;
          break;
        case 'j':
          opt.format = L"json";
          break;
        case 'h':
          usage();
          exit(0);
        case 'v':
          bela::FPrintF(stderr, L"kisasum 1.0\n");
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
    bela::FPrintF(stderr, L"no input file\n");
    return false;
  }
  opt.files = pa.UnresolvedArgs(); // fill
  return true;
}

std::optional<kisasum_result> kisasum_one_json(std::wstring_view file,
                                               belautils::algorithm::hash_t h) {
  auto filex = bela::PathAbsolute(file);
  kisasum::FileUtils fu;
  bela::error_code ec;
  if (!fu.Open(filex, ec)) {
    bela::FPrintF(stderr, L"unable open '%s' error: %s\n", filex, ec.message);
    return std::nullopt;
  }
  auto filename = kisasum::BaseName(filex);
  auto sumizer = belautils::make_sumizer(h);
  if (!sumizer) {
    bela::FPrintF(stderr, L"unable initialize hash sumizer\n");
    return std::nullopt;
  }
  unsigned char buffer[8192];
  int64_t total = 0;
  for (;;) {
    DWORD dw = 0;
    if (ReadFile(fu.P(), buffer, sizeof(buffer), &dw, nullptr) != TRUE) {
      break;
    }
    if (dw == 0) {
      break;
    }
    total += dw;
    sumizer->Update(buffer, dw);
  }
  if (total != fu.FileSize()) {
    bela::FPrintF(stderr, L"sum file hash error, file size: %d but read %d\n", fu.FileSize(),
                  total);
    return std::nullopt;
  }
  std::wstring hashhex;
  if (sumizer->Final(hashhex) != 0) {
    bela::FPrintF(stderr, L"hash sumizer unable final\n");
    return std::nullopt;
  }
  return std::make_optional(kisasum_result{filename, hashhex});
}

bool kisasum_execute_json(const kisasum_options &opt, belautils::algorithm::hash_t h) {
  bool ok = true;
  try {
    nlohmann::json j;
    j["algorithm"] = belautils::string_cast(bela::AsciiStrToUpper(opt.alg));
    j["files"] = nlohmann::json::array();
    auto &jfiles = j["files"];
    for (auto file : opt.files) {
      auto result = kisasum_one_json(file, h);
      if (!result) {
        ok = false;
        continue;
      }
      nlohmann::json sj;
      sj["name"] = bela::ToNarrow(result->filename);
      sj["hash"] = belautils::string_cast(result->hashhex);
      jfiles.emplace_back(std::move(sj));
    }
    bela::FPrintF(stdout, L"%s\n", j.dump(4)); /// output
  } catch (std::exception &e) {
    bela::FPrintF(stderr, L"unable dump json: %s\n", e.what());
    return false;
  }
  return ok;
}

void kisasum_one_text(std::wstring_view file, belautils::algorithm::hash_t h) {
  auto filex = bela::PathAbsolute(file);
  kisasum::FileUtils fu;
  bela::error_code ec;
  if (!fu.Open(filex, ec)) {
    bela::FPrintF(stderr, L"unable open '%s' error: %s\n", filex, ec.message);
    return;
  }
  auto filename = kisasum::BaseName(filex);
  auto sumizer = belautils::make_sumizer(h);
  if (!sumizer) {
    bela::FPrintF(stderr, L"unable initialize hash sumizer\n");
    return;
  }
  kisasum::ProgressBar bar;
  bar.Maximum(static_cast<uint64_t>(fu.FileSize()));
  bar.Execute();
  unsigned char buffer[8192];
  int64_t total = 0;
  for (;;) {
    DWORD dw = 0;
    if (ReadFile(fu.P(), buffer, sizeof(buffer), &dw, nullptr) != TRUE) {
      break;
    }
    if (dw == 0) {
      break;
    }
    total += dw;
    bar.Update(static_cast<uint64_t>(total));
    sumizer->Update(buffer, dw);
  }
  if (total != fu.FileSize()) {
    bar.MarkFault();
    bar.Finish();
    bela::FPrintF(stderr, L"\nsum file hash error, file size: %d but read %d\n", fu.FileSize(),
                  total);
    return;
  }
  std::wstring hashhex;
  if (sumizer->Final(hashhex) != 0) {
    bar.MarkFault();
    bar.Finish();
    bela::FPrintF(stderr, L"\nhash sumizer unable final\n");
    return;
  }
  bar.MarkCompleted();
  bar.Finish();
  bela::FPrintF(stderr, L"\x1b[2K\r");
  bela::FPrintF(stdout, L"%s %s\n", hashhex, filename);
}

bool kisasum_execute(const kisasum_options &opt) {
  auto h = belautils::lookup_algorithm(opt.alg);
  if (h == belautils::algorithm::NONE) {
    bela::FPrintF(stderr, L"unsupported hash algorithm: %s\n", opt.alg);
    return false;
  }
  if (bela::EqualsIgnoreCase(opt.format, L"JSON")) {
    return kisasum_execute_json(opt, h);
  }
  for (auto file : opt.files) {
    kisasum_one_text(file, h);
  }
  return true;
}

int wmain(int argc, wchar_t **argv) {
  kisasum_options opt;
  if (!parse_options(argc, argv, opt)) {
    return 1;
  }
  if (!kisasum_execute(opt)) {
    return 1;
  }
  return 0;
}
