///
#include "bona.hpp"
#include <hazel/zip.hpp>

namespace bona {
bool ViewZIP(bela::File &fd, size_t alen, nlohmann::json *j) {
  hazel::zip::Reader r;
  bela::error_code ec;
  if (!r.OpenReader(fd.FD(), bela::SizeUnInitialized, ec)) {
    AssignError(j, ec);
    bela::FPrintF(stderr, L"ZIP OpenReader: %s\n", ec.message);
    return false;
  }
  return true;
}

} // namespace bona