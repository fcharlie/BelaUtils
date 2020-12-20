///
#include "bona.hpp"
#include <bela/pe.hpp>
#include "writer.hpp"

namespace bona {
bool AnalysisPE(bela::File &fd, Writer &w) {
  bela::pe::File file;
  bela::error_code ec;
  if (!file.NewFile(fd.FD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"PE NewFile: %s\n", ec.message);
    return false;
  }
  return true;
}

} // namespace bona