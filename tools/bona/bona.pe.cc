///
#include "bona.hpp"
#include <bela/pe.hpp>
#include "writer.hpp"

namespace bona {

#ifndef IMAGE_FILE_MACHINE_RISCV32
#define IMAGE_FILE_MACHINE_RISCV32 0x5032
#endif
#ifndef IMAGE_FILE_MACHINE_RISCV64
#define IMAGE_FILE_MACHINE_RISCV64 0x5064
#endif
#ifndef IMAGE_FILE_MACHINE_RISCV128
#define IMAGE_FILE_MACHINE_RISCV128 0x5128
#endif

#ifndef IMAGE_FILE_MACHINE_CHPE_X86
#define IMAGE_FILE_MACHINE_CHPE_X86 0x3A64 /// defined in ntimage.h
#endif

#ifndef IMAGE_FILE_MACHINE_ARM64EC
#define IMAGE_FILE_MACHINE_ARM64EC 0xA641
#endif

#ifndef IMAGE_FILE_MACHINE_ARM64X
#define IMAGE_FILE_MACHINE_ARM64X 0xA64E
#endif

struct key_value_t {
  uint32_t index;
  const std::wstring_view value;
};

inline constexpr std::wstring_view Machine(uint32_t index) {
  // https://docs.microsoft.com/en-us/windows/desktop/Debug/pe-format#machine-types
  constexpr const key_value_t machines[] = {
      {IMAGE_FILE_MACHINE_UNKNOWN, L"UNKNOWN"},
      {IMAGE_FILE_MACHINE_TARGET_HOST, L"WoW Gest"},
      {IMAGE_FILE_MACHINE_I386, L"Intel 386"},
      {IMAGE_FILE_MACHINE_R3000, L"MIPS little-endian, 0x160 big-endian"},
      {IMAGE_FILE_MACHINE_R4000, L"MIPS little-endian"},
      {IMAGE_FILE_MACHINE_R10000, L"MIPS little-endian"},
      {IMAGE_FILE_MACHINE_WCEMIPSV2, L"MIPS little-endian WCE v2"},
      {IMAGE_FILE_MACHINE_ALPHA, L"Alpha_AXP"},
      {IMAGE_FILE_MACHINE_SH3, L"Hitachi SH3 "},
      {IMAGE_FILE_MACHINE_SH3DSP, L"Hitachi SH3 DSP"},
      {IMAGE_FILE_MACHINE_SH3E, L"Hitachi SH3E"},
      {IMAGE_FILE_MACHINE_SH4, L"Hitachi SH4"},
      {IMAGE_FILE_MACHINE_SH5, L"Hitachi SH5"},
      {IMAGE_FILE_MACHINE_ARM, L"ARM Little-Endian"},
      {IMAGE_FILE_MACHINE_THUMB, L"ARM Thumb/Thumb-2 Little-Endian"},
      {IMAGE_FILE_MACHINE_ARMNT, L"ARM Thumb-2 Little-Endian"},
      {IMAGE_FILE_MACHINE_AM33, L"Matsushita AM33 "},
      {IMAGE_FILE_MACHINE_POWERPC, L"Power PC little endian"},
      {IMAGE_FILE_MACHINE_POWERPCFP, L"Power PC with floating point support "},
      {IMAGE_FILE_MACHINE_IA64, L"Intel Itanium"},
      {IMAGE_FILE_MACHINE_MIPS16, L"MIPS"},
      {IMAGE_FILE_MACHINE_ALPHA64, L"ALPHA64"},
      {IMAGE_FILE_MACHINE_MIPSFPU, L"MIPS with FPU"},
      {IMAGE_FILE_MACHINE_MIPSFPU16, L"MIPS16 with FPU"},
      {IMAGE_FILE_MACHINE_TRICORE, L"Infineon"},
      {IMAGE_FILE_MACHINE_CEF, L"IMAGE_FILE_MACHINE_CEF"},
      {IMAGE_FILE_MACHINE_EBC, L"EFI Byte Code"},
      {IMAGE_FILE_MACHINE_AMD64, L"AMD64"},
      {IMAGE_FILE_MACHINE_M32R, L"Mitsubishi M32R little endian "},
      {IMAGE_FILE_MACHINE_ARM64, L"ARM64"},
      {IMAGE_FILE_MACHINE_CEE, L"IMAGE_FILE_MACHINE_CEE"},
      {IMAGE_FILE_MACHINE_CHPE_X86, L"Hybrid PE"},
      {IMAGE_FILE_MACHINE_ARM64EC, L"ARM64 Emulation Compatible(ARM64EC)"},
      {IMAGE_FILE_MACHINE_ARM64X, L"ARM64X"},
      {IMAGE_FILE_MACHINE_RISCV32, L"RISC-V 32-bit"},
      {IMAGE_FILE_MACHINE_RISCV64, L"RISC-V 64-bit"},
      {IMAGE_FILE_MACHINE_RISCV128, L"RISC-V 128-bit"}
      //
  };
  for (const auto &kv : machines) {
    if (kv.index == index) {
      return kv.value;
    }
  }
  return L"UNKNOWN";
}
// https://docs.microsoft.com/en-us/windows/desktop/api/winnt/ns-winnt-_image_file_header
inline std::vector<std::wstring> Characteristics(uint32_t index, uint32_t dllindex = 0) {
  std::vector<std::wstring> csv;
  const key_value_t cs[] = {
      {IMAGE_FILE_RELOCS_STRIPPED, L"Relocation info stripped"},
      // Relocation info stripped from file.
      {IMAGE_FILE_EXECUTABLE_IMAGE, L"Executable"},
      // File is executable  (i.e. no unresolved external references).
      {IMAGE_FILE_LINE_NUMS_STRIPPED, L"PE line numbers stripped"},
      // Line nunbers stripped from file.
      {IMAGE_FILE_LOCAL_SYMS_STRIPPED, L"Symbol stripped"},
      // Local symbols stripped from file.
      {IMAGE_FILE_AGGRESIVE_WS_TRIM, L"Aggressively trim the working set"},
      // Aggressively trim working set
      {IMAGE_FILE_LARGE_ADDRESS_AWARE, L"Large address aware"},
      // App can handle >2gb addresses
      {IMAGE_FILE_BYTES_REVERSED_LO, L"obsolete"},
      // Bytes of machine word are reversed.
      {IMAGE_FILE_32BIT_MACHINE, L"Support 32-bit words"},
      // 32 bit word machine.
      {IMAGE_FILE_DEBUG_STRIPPED, L"Debug info stripped"},
      // Debugging info stripped from file in .DBG file
      {IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP, L"Removable run from swap"},
      // If Image is on removable media, copy and run from the swap file.
      {IMAGE_FILE_NET_RUN_FROM_SWAP, L"Net run from swap"},
      // If Image is on Net, copy and run from the swap file.
      {IMAGE_FILE_SYSTEM, L"System"},
      // System File.
      {IMAGE_FILE_DLL, L"Dynamic Link Library"},
      // File is a DLL.
      {IMAGE_FILE_UP_SYSTEM_ONLY, L"Uni-processor only"},
      // File should only be run on a UP machine
      {IMAGE_FILE_BYTES_REVERSED_HI, L"obsolete"}
      // Bytes of machine word are reversed.
  };
  // USHORT  DllCharacteristics;
  constexpr const key_value_t dcs[] = {
      {IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA, L"High entropy VA"},
      {IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE, L"Dynamic base"},
      {IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY, L"Force integrity check"},
      {IMAGE_DLLCHARACTERISTICS_NX_COMPAT, L"NX compatible"},
      {IMAGE_DLLCHARACTERISTICS_NO_ISOLATION, L"No isolation"},
      {IMAGE_DLLCHARACTERISTICS_NO_SEH, L"No SEH"},
      {IMAGE_DLLCHARACTERISTICS_NO_BIND, L"Do not bind"},
      {IMAGE_DLLCHARACTERISTICS_APPCONTAINER, L"AppContainer"},
      {IMAGE_DLLCHARACTERISTICS_WDM_DRIVER, L"WDM driver"},
      {IMAGE_DLLCHARACTERISTICS_GUARD_CF, L"Control Flow Guard"},
      {IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE, L"Terminal server aware"}
      //
  };
  for (const auto &kv : cs) {
    if ((kv.index & index) != 0) {
      csv.emplace_back(kv.value);
    }
  }
  for (const auto &kv : dcs) {
    if ((kv.index & dllindex) != 0) {
      csv.emplace_back(kv.value);
    }
  }
  return csv;
}

inline std::wstring_view Subsystem(uint32_t index) {
  constexpr const key_value_t subs[] = {
      {IMAGE_SUBSYSTEM_UNKNOWN, L"UNKNOWN"},
      {IMAGE_SUBSYSTEM_NATIVE, L"Device drivers and native Windows processes "},
      {IMAGE_SUBSYSTEM_WINDOWS_GUI, L"Windows GUI"},
      {IMAGE_SUBSYSTEM_WINDOWS_CUI, L"Windows CUI"},
      {IMAGE_SUBSYSTEM_OS2_CUI, L"OS/2  character subsytem"},
      {IMAGE_SUBSYSTEM_POSIX_CUI, L"Posix character subsystem"},
      {IMAGE_SUBSYSTEM_NATIVE_WINDOWS, L"Native Win9x driver"},
      {IMAGE_SUBSYSTEM_WINDOWS_CE_GUI, L"Windows CE"},
      {IMAGE_SUBSYSTEM_EFI_APPLICATION, L"EFI Application"},
      {IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER, L"EFI Boot Service Driver"},
      {IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER, L"EFI Runtime Driver"},
      {IMAGE_SUBSYSTEM_EFI_ROM, L"EFI ROM image"},
      {IMAGE_SUBSYSTEM_XBOX, L"Xbox system"},
      {IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION, L"Windows Boot Application"},
      {IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG, L"XBOX Code Catalog"}
      //
  };
  for (const auto &kv : subs) {
    if (kv.index == index) {
      return kv.value;
    }
  }
  return L"UNKNOWN";
}

bool writeFuctionTableFull(const bela::pe::FunctionTable &ft, bela::pe::SymbolSearcher &sse, Writer &w) {
  if (!ft.imports.empty()) {
    w.Write(L"\x1b[36mDepends\x1b[0m");
    for (const auto &d : ft.imports) {
      w.Write(L"Depends", d.first, d.second, sse);
    }
  }
  if (!ft.delayimprots.empty()) {
    w.Write(L"\x1b[36mDelay\x1b[0m");
    for (const auto &d : ft.delayimprots) {
      w.Write(L"Delay", d.first, d.second, sse);
    }
  }
  if (ft.exports.empty()) {
    return true;
  }
  if (auto j = w.Raw(); j != nullptr) {
    auto av = nlohmann::json::array();
    for (const auto &d : ft.exports) {
      if (d.ForwardName.empty()) {
        av.push_back(nlohmann::json{{"address", d.Address},
                                    {"hint", d.Hint},
                                    {"name", d.Name},
                                    {"ordinal", d.Ordinal},
                                    {"undecoratedname", bela::demangle(d.Name)}});
        continue;
      }
      av.push_back(nlohmann::json{{"address", d.Address},
                                  {"forwardname", bela::encode_into<wchar_t, char>(bela::fromascii(d.ForwardName))},
                                  {"hint", d.Hint},
                                  {"name", d.Name},
                                  {"ordinal", d.Ordinal},
                                  {"undecoratedname", bela::demangle(d.Name)}});
    }
    j->emplace("export", std::move(av));
    return true;
  }
  for (const auto &d : ft.exports) {
    if (d.Name.empty()) {
      bela::FPrintF(stdout, L"\x1b[35mE %5d %08X (ununamed)\x1b[0m\n", d.Ordinal, d.Address);
      continue;
    }
    if (d.ForwardName.empty()) {
      bela::FPrintF(stdout, L"\x1b[35mE %5d %08X %s  (Hint: %d)\x1b[0m\n", d.Ordinal, d.Address, bela::demangle(d.Name),
                    d.Hint);
      continue;
    }
    bela::FPrintF(stdout, L"\x1b[35mE %5d %08X %s  (Hint: %d) --> %s\x1b[0m\n", d.Ordinal, d.Address,
                  bela::demangle(d.Name), d.Hint, d.ForwardName);
  }
  return true;
}

std::string checkedDemangle(const std::string &MangledName) {
  if (MangledName.empty()) {
    return "(unnamed)";
  }
  return bela::demangle(MangledName);
}

bool writeFuctionTable(const bela::pe::FunctionTable &ft, Writer &w) {
  std::vector<std::string> depends;
  for (const auto &t : ft.imports) {
    depends.emplace_back(t.first);
  }
  w.Write(L"Depends", depends);
  if (!ft.delayimprots.empty()) {
    std::vector<std::string> delay;
    for (const auto &t : ft.delayimprots) {
      delay.emplace_back(t.first);
    }
    w.Write(L"Delay", delay);
  }
  if (!ft.exports.empty()) {
    std::vector<std::string> exports;
    for (const auto &e : ft.exports) {
      exports.emplace_back(checkedDemangle(e.Name));
    }
    if (!exports.empty()) {
      w.Write(L"Exports", exports);
    }
  }
  return true;
}

// 24:
// section
constexpr const wchar_t *format =
    L"\x1b[%dm%s\x1b[0m\n  NumberOfLineNumbers:  %d\n  NumberOfRelocations:  %d\n  Characteristics:      %d\n  Offset: "
    L"              %d\n  PointerToLineNumbers: %d\n  PointerToRelocations: %d\n  Size:                 %d\n  "
    L"VirtualAddress:       %d\n  VirtualSize:          %d\n";

void writeSections(const bela::pe::File &file, Writer &w) {
  if (auto j = w.Raw(); j != nullptr) {
    auto secs = nlohmann::json::array();
    for (const auto &sec : file.Sections()) {
      secs.push_back(nlohmann::json{
          {"Name", sec.Name},
          {"NumberOfLineNumbers", sec.NumberOfLineNumbers},
          {"NumberOfRelocations", sec.NumberOfRelocations},
          {"Characteristics", sec.Characteristics},
          {"Offset", sec.Offset},
          {"PointerToLineNumbers", sec.PointerToLineNumbers},
          {"PointerToRelocations", sec.PointerToRelocations},
          {"Size", sec.Size},
          {"VirtualAddress", sec.VirtualAddress},
          {"VirtualSize", sec.VirtualSize},
      });
    }
    j->emplace("section", std::move(secs));
    return;
  }

  bela::FPrintF(stdout, L"Sections:\n");
  const int colors[] = {34, 35, 36};
  size_t i = 0;
  for (const auto &sec : file.Sections()) {
    bela::FPrintF(stdout, format, colors[i % std::size(colors)], sec.Name, sec.NumberOfLineNumbers,
                  sec.NumberOfRelocations, sec.Characteristics, sec.Offset, sec.PointerToLineNumbers,
                  sec.PointerToRelocations, sec.Size, sec.VirtualAddress, sec.VirtualSize);
    i++;
  }
}

bool AnalysisOverlay(bela::io::FD &fd, int64_t overlayOffset, Writer &w) {
  hazel::hazel_result hr;
  bela::error_code ec;
  if (!hazel::LookupFile(fd, hr, ec, overlayOffset)) {
    return false;
  }
  w.Write(L"Overlay", hr.description());
  if (hr.LooksLikeZIP()) {
    AnalysisZIP(fd, w, overlayOffset);
  }
  return true;
}

bool AnalysisPE(bela::io::FD &fd, Writer &w) {
  bela::pe::File file;
  bela::error_code ec;
  if (!file.NewFile(fd.NativeFD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"PE NewFile: %s\n", ec.message);
    return false;
  }
  w.Write(L"Machine", Machine(file.Fh().Machine));
  w.WriteBool(L"Is64Bit", file.Is64Bit());
  std::wstring fullPath;
  if (auto fullPath_ = bela::RealPathByHandle(fd.NativeFD(), ec); fullPath_) {
    fullPath = std::move(*fullPath_);
  }
  bela::pe::SymbolSearcher sse(fullPath, file.Machine());
  w.Write(L"Subsystem", Subsystem(static_cast<uint32_t>(file.Subsystem())));
  auto cs = Characteristics(file.Fh().Characteristics, file.Header().DllCharacteristics);
  w.Write(L"Characteristic", cs);
  std::string clrver;
  if (auto meta = file.LookupDotNetMetadata(ec); meta) {
    w.Write(L"CLRVersion", meta->version);
    w.Write(L"CLRFlags", meta->flags);
  }
  // show overlay bytes
  if (auto overlayLen = file.OverlayLength(); overlayLen > 0) {
    w.Write(L"OverlayLen", overlayLen);
    if (overlayLen > 512) {
      AnalysisOverlay(fd, file.OverlayOffset(), w);
    }
  }
  if (IsFullMode) {
    writeSections(file, w);
  }
  bela::pe::FunctionTable ft;
  if (file.LookupFunctionTable(ft, ec)) {
    if (IsFullMode) {
      writeFuctionTableFull(ft, sse, w);
    } else {
      writeFuctionTable(ft, w);
    }
  }
  if (auto j = w.Raw(); j != nullptr) {
    std::vector<bela::pe::Symbol> syms;
    if (file.LookupSymbols(syms, ec)) {
      auto av = nlohmann::json::array();
      for (const auto &s : syms) {
        av.push_back(nlohmann::json{
            {"name", s.Name},
            {"sectionnumber", s.SectionNumber},
            {"storageclass", s.StorageClass},
            {"type", s.Type},
            {"value", s.Value},
        });
      }
      j->emplace("symbol", std::move(av));
    }
  }
  return true;
}

} // namespace bona