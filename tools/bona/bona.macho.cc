///
#include "bona.hpp"
#include "writer.hpp"
#include <hazel/macho.hpp>
#include <bela/terminal.hpp>

namespace bona {
namespace internal {
using namespace hazel::macho;
constexpr const intName cpuStrings[] = {
    {CPU_TYPE_I386, "i386"},   {CPU_TYPE_X86_64, "x86_64"}, {CPU_TYPE_ARM, "arm"},
    {CPU_TYPE_ARM64, "arm64"}, {CPU_TYPE_POWERPC, "ppc"},   {CPU_TYPE_POWERPC64, "ppc64"},
};

constexpr const intName typeStrings[] = {
    {1, "Obj"},
    {2, "Exec"},
    {6, "Dylib"},
    {8, "Bundle"},
};

constexpr const intName cmdStrings[] = {
    {static_cast<int>(LoadCmd::LoadCmdSegment), "LoadCmdSegment"},
    {static_cast<int>(LoadCmd::LoadCmdThread), "LoadCmdThread"},
    {static_cast<int>(LoadCmd::LoadCmdUnixThread), "LoadCmdUnixThread"},
    {static_cast<int>(LoadCmd::LoadCmdDylib), "LoadCmdDylib"},
    {static_cast<int>(LoadCmd::LoadCmdSegment64), "LoadCmdSegment64"},
    {static_cast<int>(LoadCmd::LoadCmdRpath), "LoadCmdRpath"},
};

} // namespace internal

bool AnalysisMachO(hazel::macho::File &file, Writer &w) {
  w.Write(L"CPU", internal::stringName(file.Fh().Cpu, internal::cpuStrings));
  w.Write(L"Type", internal::stringName(file.Fh().Type, internal::typeStrings));
  w.Write(L"SubCPU", file.Fh().SubCpu);
  w.Write(L"Ncmd", file.Fh().Ncmd);
  w.Write(L"Flags", file.Fh().Flags);
  std::vector<std::string> libs;
  bela::error_code ec;
  if (file.Depends(libs, ec)) {
    w.Write(L"Depends", libs);
  }
  if (!IsFullMode) {
    return true;
  }
  std::vector<std::string> symbols;
  if (file.ImportedSymbols(symbols, ec)) {
    for (auto &s : symbols) {
      s = bela::demangle(s); // demangle
    }
    w.Write(L"Imported", symbols);
  }
  return true;
}

bool AnalysisMachO(bela::File &fd, Writer &w) {
  hazel::macho::FatFile fatfile;
  bela::error_code ec;
  if (fatfile.NewFile(fd.FD(), bela::SizeUnInitialized, ec)) {
    auto j = w.Raw();
    if (j == nullptr) {
      for (auto &a : fatfile.Arches()) {
        bela::FPrintF(stdout, L"FatMacho: %s SubCPU: %d Align: %d Offset: %d Size: %d\n",
                      internal::stringName(a.fh.Cpu, internal::cpuStrings), a.fh.SubCpu, a.fh.Align, a.fh.Offset,
                      a.fh.Size);

        AnalysisMachO(a.file, w);
      }
    } else {
      for (auto &a : fatfile.Arches()) {
        nlohmann::json sj;
        sj.emplace("fatheader", nlohmann::json{
                                    {"cpu", a.fh.Cpu},
                                    {"subcpu", a.fh.SubCpu},
                                    {"align", a.fh.Align},
                                    {"offset", a.fh.Offset},
                                    {"size", a.fh.Size},
                                });
        auto jr = j->emplace(internal::stringName(a.fh.Cpu, internal::cpuStrings), std::move(sj));
        if (!jr.second) {
          return true;
        }
        JsonWriter jw(&(jr.first.value()));
        AnalysisMachO(a.file, jw);
      }
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