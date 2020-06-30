// hastyhex fork from https://github.com/skeeto/hastyhex
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <clocale>
#include <algorithm>
#include <wchar.h>
#include <bela/terminal.hpp>
#include <bela/base.hpp>
#include <bela/numbers.hpp>
#include <bela/parseargv.hpp>
#include <belautilsversion.h>

static const constexpr char hex[] = "0123456789abcdef";

static int color(int b) {
  constexpr unsigned char CN = 0x37; /* null    */
  constexpr unsigned char CS = 0x92; /* space   */
  constexpr unsigned char CP = 0x96; /* print   */
  constexpr unsigned char CC = 0x95; /* control */
  constexpr unsigned char CH = 0x93; /* high    */
  static constexpr const unsigned char table[] = {
      CN, CC, CC, CC, CC, CC, CC, CC, CC, CC, CS, CS, CS, CS, CC, CC, CC, CC, CC, CC, CC, CC,
      CC, CC, CC, CC, CC, CC, CC, CC, CC, CC, CS, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
      CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
      CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
      CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP,
      CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CP, CC, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH,
      CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH, CH};
  return table[b];
}

static int display(int b) {
  static constexpr const char table[] = {
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
      0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
      0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
      0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
      0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
      0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
      0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
      0x2e,
  };
  return table[b];
}

static void process_color(FILE *in, FILE *out, int64_t len) {
  size_t i, n;
  unsigned long offset = 0;
  unsigned char input[16] = {0};
  constexpr const uint64_t inputlen = sizeof(input);
  char colortemplate[] = "00000000  "
                         "\33[XXm## \33[XXm## \33[XXm## \33[XXm## "
                         "\33[XXm## \33[XXm## \33[XXm## \33[XXm##  "
                         "\33[XXm## \33[XXm## \33[XXm## \33[XXm## "
                         "\33[XXm## \33[XXm## \33[XXm## \33[XXm##  "
                         "\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm."
                         "\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm.\33[XXm."
                         "\33[0m\n";
  static const int slots[] = {/* ANSI-color, hex, ANSI-color, ASCII */
                              12,  15,  142, 145, 20,  23,  148, 151, 28,  31,  154, 157, 36,
                              39,  160, 163, 44,  47,  166, 169, 52,  55,  172, 175, 60,  63,
                              178, 181, 68,  71,  184, 187, 77,  80,  190, 193, 85,  88,  196,
                              199, 93,  96,  202, 205, 101, 104, 208, 211, 109, 112, 214, 217,
                              117, 120, 220, 223, 125, 128, 226, 229, 133, 136, 232, 235};
  uint64_t maxlen = len > 0 ? len : UINT64_MAX;

  do {
    auto rn = (std::min)(maxlen, inputlen);
    n = fread(input, 1, (int)rn, in);
    maxlen -= n;
    /* Write the offset */
    for (i = 0; i < 8; i++) {
      colortemplate[i] = hex[(offset >> (28 - i * 4)) & 15];
    }

    /* Fill out the colortemplate */
    for (i = 0; i < 16; i++) {
      /* Use a fixed loop count instead of "n" to encourage loop
       * unrolling by the compiler. Empty bytes will be erased
       * later.
       */
      int v = input[i];
      int c = color(v);
      colortemplate[slots[i * 4 + 0] + 0] = hex[c >> 4];
      colortemplate[slots[i * 4 + 0] + 1] = hex[c & 15];
      colortemplate[slots[i * 4 + 1] + 0] = hex[v >> 4];
      colortemplate[slots[i * 4 + 1] + 1] = hex[v & 15];
      colortemplate[slots[i * 4 + 2] + 0] = hex[c >> 4];
      colortemplate[slots[i * 4 + 2] + 1] = hex[c & 15];
      colortemplate[slots[i * 4 + 3] + 0] = display(v);
    }

    /* Erase any trailing bytes */
    for (i = n; i < 16; i++) {
      /* This loop is only used once: the last line of output. The
       * branch predictor will quickly learn that it's never taken.
       */
      colortemplate[slots[i * 4 + 0] + 0] = '0';
      colortemplate[slots[i * 4 + 0] + 1] = '0';
      colortemplate[slots[i * 4 + 1] + 0] = ' ';
      colortemplate[slots[i * 4 + 1] + 1] = ' ';
      colortemplate[slots[i * 4 + 2] + 0] = '0';
      colortemplate[slots[i * 4 + 2] + 1] = '0';
      colortemplate[slots[i * 4 + 3] + 0] = ' ';
    }

    if (!fwrite(colortemplate, sizeof(colortemplate) - 1, 1, out)) {
      break; /* Output error */
    }
    offset += 16;
  } while (n == 16 && maxlen > 0);
}

static void process_plain(FILE *in, FILE *out, int64_t len) {
  size_t i, n;
  unsigned long offset = 0;
  unsigned char input[16] = {0};
  constexpr const uint64_t inputlen = sizeof(input);
  char colortemplate[] = "00000000  ## ## ## ## ## ## ## ##  ## ## ## ## ## ## ## ##  "
                         "................\n";
  static const int slots[] = {10, 60, 13, 61, 16, 62, 19, 63, 22, 64, 25, 65, 28, 66, 31, 67,
                              35, 68, 38, 69, 41, 70, 44, 71, 47, 72, 50, 73, 53, 74, 56, 75};

  uint64_t maxlen = len > 0 ? len : UINT64_MAX;
  do {
    auto rn = (std::min)(maxlen, inputlen);
    n = fread(input, 1, (int)rn, in);
    maxlen -= n;

    /* Write the offset */
    for (i = 0; i < 8; i++) {
      colortemplate[i] = hex[(offset >> (28 - i * 4)) & 15];
    }

    /* Fill out the template */
    for (i = 0; i < 16; i++) {
      int v = input[i];
      colortemplate[slots[i * 2 + 0] + 0] = hex[v >> 4];
      colortemplate[slots[i * 2 + 0] + 1] = hex[v & 15];
      colortemplate[slots[i * 2 + 1] + 0] = display(v);
    }

    /* Erase any trailing bytes */
    for (i = n; i < 16; i++) {
      colortemplate[slots[i * 2 + 0] + 0] = ' ';
      colortemplate[slots[i * 2 + 0] + 1] = ' ';
      colortemplate[slots[i * 2 + 1] + 0] = ' ';
    }

    if (!fwrite(colortemplate, sizeof(colortemplate) - 1, 1, out)) {
      break;
    }
    offset += 16;
  } while (n == 16 && maxlen > 0);
}

inline bool enablevtmode() {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE) {
    return false;
  }

  DWORD dwMode = 0;
  if (!GetConsoleMode(hOut, &dwMode)) {
    return false;
  }

  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!SetConsoleMode(hOut, dwMode)) {
    return false;
  }
  return true;
}

const wchar_t *tlsstrerror(int ec) {
  static thread_local wchar_t buffer[512];
  if (_wcserror_s(buffer, ec) != 0) {
    return L"unkown error";
  }
  return buffer;
}

struct Options {
  std::wstring file;
  std::wstring out;
  int64_t length{-1};
  uint64_t seek{0};
  bool plaintext{false};
};

void Usage() {
  constexpr const char *ua = R"(OVERVIEW: hastyhex a faster hex dumper
Usage: hastyhex [options] <input>
OPTIONS:
  -h [--help]                      Print hastyhex usage information and exit
  -v [--version]                   Print hastyhex version and exit
  -n [--length]                    Read only N bytes from the input.
  -s [--seek]                      Read from the specified offset
  -o [--out]                       Output to file instead of standard output
  -p [--plain-text]                Do not output color ("plain")

Example:
  hastyhex file.exe

)";
  printf("%s", ua);
}

bool ParseArgv(int argc, wchar_t **argv, Options &bo) {
  bela::ParseArgv pv(argc, argv);
  pv.Add(L"help", bela::no_argument, L'h')
      .Add(L"version", bela::no_argument, 'v')
      .Add(L"length", bela::required_argument, L'n')
      .Add(L"seek", bela::required_argument, L's')
      .Add(L"plain", bela::no_argument, L'p')
      .Add(L"out", bela::required_argument, L'o');
  bela::error_code ec;
  auto result = pv.Execute(
      [&](int ch, const wchar_t *oa, const wchar_t *) {
        switch (ch) {
        case 'h':
          Usage();
          exit(0);
        case 'v':
          bela::FPrintF(stdout, L"%s\n", BELAUTILS_VERSION);
          exit(0);
        case 'o':
          bo.out = oa;
          break;
        case 'n':
          if (int64_t n = 0; bela::SimpleAtoi(oa, &n)) {
            bo.length = n;
          }
          break;
        case 's':
          if (int64_t n = 0; bela::SimpleAtoi(oa, &n)) {
            bo.seek = n;
          }
          break;
        case 'p':
          bo.plaintext = true;
          break;
        default:
          return false;
        }
        return true;
      },
      ec);

  if (!result) {
    bela::FPrintF(stderr, L"ParseArgv: %s\n", ec.message);
    return false;
  }
  if (pv.UnresolvedArgs().empty()) {
    bela::FPrintF(stderr, L"Too few arguments\n");
    return false;
  }
  bo.file = pv.UnresolvedArgs()[0];
  return true;
}

int wmain(int argc, wchar_t *argv[]) {
  enablevtmode();
  FILE *in = stdin;
  FILE *out = stdout;
  Options bo;
  if (!ParseArgv(argc, argv, bo)) {
    return 1;
  }
  if (_wfopen_s(&in, bo.file.data(), L"rb") != 0) {
    auto ec = bela::make_system_error_code();
    bela::FPrintF(stderr, L"hastyhex: open '%s' for read: %s\n", bo.file, ec.message);
    return 1;
  }
  auto closer = bela::finally([&] {
    if (in != stdin) {
      fclose(in);
    }
    if (out != stdout) {
      fclose(out);
    }
  });
  if (!bo.out.empty()) {
    if (_wfopen_s(&out, bo.out.data(), L"wb") != 0) {
      auto ec = bela::make_system_error_code();
      bela::FPrintF(stderr, L"hastyhex: open '%s' for write: %s\n", bo.out, ec.message);
      return 1;
    }
  }
  if (in != stdin) {
    _fseeki64(in, bo.seek, SEEK_SET);
  }
  if (bo.plaintext) {
    process_plain(in, out, bo.length);
    return 0;
  }
  process_color(in, out, bo.length);
  return 0;
}
