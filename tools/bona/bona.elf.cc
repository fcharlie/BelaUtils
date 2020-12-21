///
#include "bona.hpp"
#include "writer.hpp"
#include <hazel/elf.hpp>

namespace bona {
bool AnalysisELF(bela::File &fd, Writer &w) {
  hazel::elf::File file;
  bela::error_code ec;
  if (!file.NewFile(fd.FD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"unable new ELF file %s\n", ec.message);
    return false;
  }
  //
  return true;
}

} // namespace bona