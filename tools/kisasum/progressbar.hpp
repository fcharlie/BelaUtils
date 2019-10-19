//
#ifndef KISASUM_PROGRESSBAR_HPP
#define KISASUM_PROGRESSBAR_HPP
#include <bela/stdwriter.hpp>
#include <bela/base.hpp>
#include <chrono>

namespace kisasum {

inline uint32_t TerminalWidth() {
  auto h = GetStdHandle(STD_ERROR_HANDLE);
  if (h == nullptr || h == INVALID_HANDLE_VALUE) {
    return 80;
  }
  if (GetFileType(h) != FILE_TYPE_CHAR) {
    // Only support Mintty (TERM not VT100)
    // https://wiki.archlinux.org/index.php/working_with_the_serial_console#Resizing_a_terminal
    // write escape string detect col
    // result like ';120R'
    // fprintf(stderr, "\x1b7\x1b[r\x1b[999;999H\x1b[6n\x1b8");

    return 90;
  }
  CONSOLE_SCREEN_BUFFER_INFO bi;
  if (GetConsoleScreenBufferInfo(h, &bi) != TRUE) {
    return 80;
  }
  return bi.dwSize.X;
}

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
    width = TerminalWidth();
    space.resize(width, L' ');
    scs.resize(width, L'#');
    start_time = tick;
  }
  void Update(int64_t bytes) {
    std::wstring_view svspace(space);
    std::wstring_view svchars(scs);
    auto k = width - 30;
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
      bela::FPrintF(stdout, L"\r[%s%s%s] %s %d%% completed.",
                    svchars.substr(0, X), arrow, svspace.substr(0, Y), buf, N);
    }
    tick = now;
  }
  void Refresh() {
    //
    bela::FPrintF(stderr, L"\r%s", space);
  }

private:
  std::wstring space;
  std::wstring scs;
  int64_t total{0};
  int64_t completed{0};
  uint32_t width{0};
  std::chrono::steady_clock::time_point start_time;
  std::chrono::steady_clock::time_point tick;
};
} // namespace kisasum

#endif
