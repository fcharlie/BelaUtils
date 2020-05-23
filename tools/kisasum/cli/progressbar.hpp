//
#ifndef KISASUM_PROGRESSBAR_HPP
#define KISASUM_PROGRESSBAR_HPP
#include <bela/terminal.hpp>
#include <bela/base.hpp>
#include <bela/process.hpp>
#include <bela/str_split.hpp>
#include <bela/ascii.hpp>
#include <chrono>

namespace kisasum {

class ProgressBar {
public:
  enum : uint64_t {
    KB = 1024,
    XKB = 10 * 1024,
    MB = 1024 * 1024,
    GB = 1024ULL * 1024 * 1024
  };
  ProgressBar() = default;
  ProgressBar(const ProgressBar &) = delete;
  ProgressBar &operator=(const ProgressBar &) = delete;
  void Initialize(int64_t to) {
    total = to;
    completed = 0;
    tick = std::chrono::steady_clock::now();
    if (!InitializeInternal()) {
      return;
    }
    initialized = true;
    start_time = tick;
    space.resize(termsz.columns, L' ');
    scs.resize(termsz.columns, L'#');
  }
  void Update(int64_t bytes) {
    std::wstring_view svspace(space);
    std::wstring_view svchars(scs);
    auto k = termsz.columns - 30;
    completed += bytes;
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - tick)
            .count() > 10) {
      auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(
          tick - start_time);
      auto speed = completed * 1000'000'000 / diff.count();
      auto N = completed * 100 / total;
      auto X = N * k / 100;
      std::wstring_view arrow;
      auto Y = k - X;
      if (N % 2 != 0) {
        arrow = L">";
        Y--;
      }
      wchar_t buf[64];
      if (speed > GB) {
        _snwprintf_s(buf, 64, L"%4.2f GB/s ", (double)speed / GB);
      } else if (speed > MB) {
        _snwprintf_s(buf, 64, L"%4.2f MB/s ", (double)speed / MB);
      } else if (speed > XKB) {
        _snwprintf_s(buf, 64, L"%4.2f KB/s ", (double)speed / KB);
      } else {
        _snwprintf_s(buf, 64, L"%lld B ", speed);
      }
      bela::FPrintF(stdout, L"\x1b[2K\r[%s%s%s] %s %d%% completed.",
                    svchars.substr(0, X), arrow, svspace.substr(0, Y), buf, N);
    }
    tick = now;
  }
  void Refresh() {
    if (initialized) {
      return;
    }
    bela::FPrintF(stderr, L"\r%s", space);
  }

private:
  bool CygwinTerminalSize() {
    bela::process::Process ps;
    constexpr DWORD flags =
        bela::process::CAPTURE_USEIN | bela::process::CAPTURE_USEERR;
    if (auto exitcode = ps.CaptureWithMode(flags, L"stty", L"size");
        exitcode != 0) {
      bela::FPrintF(stderr, L"stty %d: %s\n", exitcode, ps.ErrorCode().message);
      return false;
    }
    auto out = bela::ToWide(ps.Out());
    std::vector<std::wstring_view> ss =
        bela::StrSplit(bela::StripTrailingAsciiWhitespace(out),
                       bela::ByChar(' '), bela::SkipEmpty());
    if (ss.size() != 2) {
      return false;
    }
    if (!bela::SimpleAtoi(ss[0], &termsz.rows)) {
      return false;
    }
    if (!bela::SimpleAtoi(ss[1], &termsz.columns)) {
      return false;
    }
    if (termsz.columns < 80) {
      termsz.columns = 80;
    }
    return true;
  }
  bool InitializeInternal() {
    if (bela::terminal::IsTerminal(stderr)) {
      bela::terminal::TerminalSize(stderr, termsz);
      return true;
    }
    if (!bela::terminal::IsCygwinTerminal(stderr)) {
      return false;
    }
    return true;
  }
  std::wstring space;
  std::wstring scs;
  bool initialized{false};
  int64_t total{0};
  int64_t completed{0};
  bela::terminal::terminal_size termsz;
  std::chrono::steady_clock::time_point start_time;
  std::chrono::steady_clock::time_point tick;
};
} // namespace kisasum

#endif
