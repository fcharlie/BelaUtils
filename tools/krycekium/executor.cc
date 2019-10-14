// executor.cc
#include "executor.hpp"

namespace krycekium {
bool Executor::PushEvent(const std::wstring &msi, const std::wstring &outdir,
                         void *data) {
  (void)msi;
  (void)outdir;
  (void)data;
  return false;
}
} // namespace krycekium
