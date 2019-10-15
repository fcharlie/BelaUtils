///
#ifndef KRYCEKIUM_EXECUTOR_HPP
#define KRYCEKIUM_EXECUTOR_HPP
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
  std::wstring msi;
  std::wstring outdir;
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
  bool PushEvent(const std::wstring &msi, const std::wstring &outdir,
                 void *data);
  // cancel current task
  void Cancel() { canceled = true; }

private:
  void run();
  bool empty();
  std::shared_ptr<std::thread> t;
  std::atomic_bool exited{false};
  std::atomic_bool canceled{false};
  std::condition_variable cv;
  std::mutex mtx;
  std::queue<Packet> packets;
};

} // namespace krycekium

#endif
