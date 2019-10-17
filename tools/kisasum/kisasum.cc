///
#include <bela/parseargv.hpp>
#include <bela/stdwriter.hpp>
#include "../../lib/hashlib/sumizer.hpp"

void usage() {
  const std::wstring_view ua = LR"(OVERVIEW: kisasum 1.0
USAGE: kisasum [options] <input>
OPTIONS:
  -a, --algorithm  Hash Algorithm,support algorithm described below.
                   Algorithm Ignore case, default sha256
  -f, --format     Return information about hash in a format described below.
  -h, -?, --help   Print usage and exit.
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

int wmain(int argc, wchar_t **argv) {
  usage();
  (void)argc;
  (void)argv;
  return 0;
}
