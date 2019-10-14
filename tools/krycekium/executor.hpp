///
#ifndef KRYCEKIUM_EXECUTOR_HPP
#define KRYCEKIUM_EXECUTOR_HPP
#include <atomic>
#include <condition_variable>
#include <memory>
#include <string>
#include <thread>

namespace krycekium {
class Executor {
public:
  Executor() = default;
  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;
  bool PushEvent(const std::wstring &msi, const std::wstring &outdir,
                 void *data);

  // callback real
  int Callback(uint32_t type, const wchar_t *msg);
  void Cancel() { canceled = true; }

private:
  std::shared_ptr<std::thread> t;
  std::atomic_bool initialized{false};
  std::atomic_bool canceled{false};
  std::condition_variable cv;
};

} // namespace krycekium

#endif
