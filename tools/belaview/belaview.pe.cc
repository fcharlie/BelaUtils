///
#include "belaview.hpp"

namespace belaview {
bool ViewPE(bela::File &fd, size_t alen, nlohmann::json *j) {
  bela::pe::File file;
  bela::error_code ec;
  if (!file.NewFile(fd.FD(), bela::SizeUnInitialized, ec)) {
    AssignError(j, ec);
    bela::FPrintF(stderr, L"PE NewFile: %s\n", ec.message);
    return false;
  }
  if (j != nullptr) {
    //
    return true;
  }
  return true;
}

} // namespace belaview