///
#ifndef KRYCEKIUM_EXECUTOR_HPP
#define KRYCEKIUM_EXECUTOR_HPP
#include "krycekium.hpp"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <string>
#include <queue>
#include <thread>

namespace krycekium {
enum class Status {
  None,
  Completed,
  Failure //
};
// task Packet
struct Packet {
  Packet() = default;
  Packet(std::wstring_view m, std::wstring_view o, HWND w, HWND p)
      : msi(m), outdir(o), hWnd(w), hProgress(p) {}
  Packet(Packet &&other) {
    msi = std::move(other.msi);
    outdir = std::move(other.outdir);
    hWnd = other.hWnd;
    hProgress = other.hProgress;
    other.hWnd = nullptr;
    other.hProgress = nullptr;
  }
  Packet &operator=(Packet &&other) {
    msi = std::move(other.msi);
    outdir = std::move(other.outdir);
    hWnd = other.hWnd;
    hProgress = other.hProgress;
    other.hWnd = nullptr;
    other.hProgress = nullptr;
    return *this;
  }
  std::wstring msi;
  std::wstring outdir;
  HWND hWnd{nullptr};
  HWND hProgress{nullptr};
};

// single thread executor
class Executor {
public:
  Executor() = default;
  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;
  //
  ~Executor() {
    exited = true;
    cv.notify_all();
    if (t) {
      t->join();
    }
  }
  bool InitializeExecutor();
  bool PushEvent(std::wstring_view msi, std::wstring_view outdir, HWND hWnd,
                 HWND hProgress);
  // cancel current task
  void Cancel() { canceled = true; }
  bool IsCanceled() const { return canceled; }

private:
  bool execute(Packet &pk);
  void run();
  std::shared_ptr<std::thread> t;
  std::atomic_bool exited{false};
  std::atomic_bool canceled{false};
  std::condition_variable_any cv;
  std::recursive_mutex mtx;
  std::queue<Packet> packets;
};

} // namespace krycekium

#endif
