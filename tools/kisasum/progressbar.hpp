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
    start_time = std::chrono::steady_clock::now();
  }
  void Update(int64_t bytes) {
    //
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
};
} // namespace kisasum

#endif