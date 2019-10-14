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

private:
  std::shared_ptr<std::thread> t;
  std::condition_variable cv;
};

} // namespace krycekium

#endif
