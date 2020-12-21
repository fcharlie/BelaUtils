///
#include "bona.hpp"
#include "writer.hpp"
#include <hazel/macho.hpp>

namespace bona {
bool AnalysisMachO(bela::File &fd, Writer &w) {
  hazel::macho::File file;
  bela::error_code ec;
  if (!file.NewFile(fd.FD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"unable new Mach-O file %s\n", ec.message);
    return false;
  }
  //
  return true;
}

} // namespace bona