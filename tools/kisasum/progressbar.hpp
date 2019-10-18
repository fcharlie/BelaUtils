//
#ifndef KISASUM_PROGRESSBAR_HPP
#define KISASUM_PROGRESSBAR_HPP
#include <bela/stdwriter.hpp>
#include <chrono>

namespace kisasum {
class ProgressBar {
public:
  ProgressBar() = default;
  ProgressBar(const ProgressBar &) = delete;
  ProgressBar &operator=(const ProgressBar &) = delete;
  void Initialize(int64_t to) {
    total = to;
    completed = 0;
    tick = std::chrono::steady_clock::now();
    start_time = tick;
  }
  void Update(int64_t bytes) {
    auto now = std::chrono::steady_clock::now();
    completed += bytes;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - tick)
            .count() > 10) {
      auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(
          tick - start_time);
      auto speed = completed * 1000'000'000 / diff.count();
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
  std::chrono::steady_clock::time_point start_time;
  std::chrono::steady_clock::time_point tick;
};
} // namespace kisasum

#endif
