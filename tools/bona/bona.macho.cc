///
#include "bona.hpp"
#include "writer.hpp"
#include <hazel/macho.hpp>

namespace bona {
bool AnalysisMachO(hazel::macho::File &file, Writer &w) {
  //
  return true;
}

bool AnalysisMachO(bela::File &fd, Writer &w) {
  hazel::macho::FatFile fatfile;
  bela::error_code ec;
  if (fatfile.NewFile(fd.FD(), bela::SizeUnInitialized, ec)) {
    for (auto &a : fatfile.Arches()) {
    }
    return true;
  }
  if (ec.code != hazel::macho::ErrNotFat) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"unable new Mach-O file %s\n", ec.message);
    return false;
  }
  ec.clear();
  hazel::macho::File file;
  if (!file.NewFile(fd.FD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"unable new Mach-O file %s\n", ec.message);
    return false;
  }
  return AnalysisMachO(file, w);
}

} // namespace bona