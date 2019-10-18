///
#include <bela/parseargv.hpp>
#include <bela/stdwriter.hpp>
#include <vector>
#include "../../lib/hashlib/sumizer.hpp"

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
  MD5        SHA1
  SHA224     SHA256     SHA384     SHA512
  SHA3-224   SHA3-256   SHA3-384   SHA3-512
  BLAKE2s    BLAKE2b

Formats:
  text     format to text, support progress
  json     format to json
)";
  bela::FPrintF(stderr, L"%s\n", ua);
}

struct kisasum_options {
  std::wstring_view alg{L"SHA256"};
  std::wstring_view format{L"json"};
  std::vector<std::wstring_view> files;
};

bool parse_options(int argc, wchar_t **argv, kisasum_options &opt) {
  bela::ParseArgv pa(argc, argv);
  pa.Add(L"algorithm", bela::required_argument, 'a')
      .Add(L"format", bela::required_argument, 'f')
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

bool sumfiles(const kisasum_options &opt) {
  auto h = belautils::lookup_algorithm(opt.alg);
  if (h == belautils::algorithm::NONE) {
    bela::FPrintF(stderr, L"unsupported hash algorithm: %s\n", opt.alg);
    return false;
  }
  return true;
}

int wmain(int argc, wchar_t **argv) {
  kisasum_options opt;
  if (!parse_options(argc, argv, opt)) {
    return 1;
  }
  return 0;
}
