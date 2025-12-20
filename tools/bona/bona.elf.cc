///
#include "bona.hpp"
#include "writer.hpp"
#include <hazel/elf.hpp>

#ifndef ELFOSABI_86OPEN
#define ELFOSABI_86OPEN 5
#endif

namespace bona {
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format

namespace internal {
using namespace hazel::elf;
static constexpr const intName osabiName[] = {
    {ELFOSABI_NONE, "UNIX System V ABI"},
    {ELFOSABI_HPUX, "HP-UX operating system"},
    {ELFOSABI_NETBSD, "NetBSD"},
    {ELFOSABI_GNU, "GNU/Linux"},
    {ELFOSABI_LINUX, "Historical alias for ELFOSABI_GNU"},
    {ELFOSABI_HURD, "GNU/Hurd"},
    {ELFOSABI_SOLARIS, "Solaris"},
    {ELFOSABI_AIX, "AIX"},
    {ELFOSABI_IRIX, "IRIX"},
    {ELFOSABI_FREEBSD, "FreeBSD"},
    {ELFOSABI_TRU64, "TRU64 UNIX"},
    {ELFOSABI_MODESTO, "Novell Modesto"},
    {ELFOSABI_OPENBSD, "OpenBSD"},
    {ELFOSABI_OPENVMS, "OpenVMS"},
    {ELFOSABI_NSK, " Hewlett-Packard Non-Stop Kernel"},
    {ELFOSABI_AROS, "AROS"},
    {ELFOSABI_FENIXOS, "FenixOS"},
    {ELFOSABI_CLOUDABI, "Nuxi CloudABI"},
    {ELFOSABI_CUDA, "NVIDIA CUDA architecture"},
    {ELFOSABI_CUDA_V2, "NVIDIA CUDA architecture"},
    {ELFOSABI_FIRST_ARCH, "First architecture-specific OS ABI"},
    {ELFOSABI_AMDGPU_HSA, "AMD HSA runtime"},
    {ELFOSABI_AMDGPU_PAL, "AMD PAL runtime"},
    {ELFOSABI_AMDGPU_MESA3D, "AMD GCN GPUs (GFX6+) for MESA runtime"},
    {ELFOSABI_ARM, "ARM"},
    {ELFOSABI_ARM_FDPIC, "ARM FDPIC"},
    {ELFOSABI_C6000_ELFABI, "Bare-metal TMS320C6000"},
    {ELFOSABI_C6000_LINUX, "Linux TMS320C6000"},
    {ELFOSABI_STANDALONE, "Standalone (embedded) application"},
    {ELFOSABI_LAST_ARCH, "Last Architecture-specific OS ABI"},
};

static constexpr const intName amdgpuelfabi[] = {
    {ELFOSABI_AMDGPU_HSA, "AMDGPU - HSA"},
    {ELFOSABI_AMDGPU_PAL, "AMDGPU - PAL"},
    {ELFOSABI_AMDGPU_MESA3D, "AMDGPU - MESA3D"},
};

static constexpr const intName c6000elfabi[] = {
    {ELFOSABI_C6000_ELFABI, "Bare-metal C6000"},
    {ELFOSABI_C6000_LINUX, "Linux C6000"},
};

static constexpr const intName typeStrings[] = {
    {0, "NONE"},         {1, "Relocatable file"}, {2, "Executable file"}, {3, "Shared object file"}, {4, "Core file"},
    {0xfe00, "ET_LOOS"}, {0xfeff, "ET_HIOS"},     {0xff00, "ET_LOPROC"},  {0xffff, "ET_HIPROC"},
};

static constexpr const intName machines[] = {
    {EM_NONE, "No machine"},
    {EM_M32, "AT&T WE 32100"},
    {EM_SPARC, "Sparc"},
    {EM_386, "Intel 80386"},
    {EM_68K, "Motorola 68000"},
    {EM_88K, "Motorola 88000"},
    {EM_IAMCU, "Intel MCU"},
    {EM_860, "Intel 80860"},
    {EM_MIPS, "MIPS R3000"},
    {EM_S370, "IBM System/370"},
    {EM_MIPS_RS3_LE, "MIPS R3000 little-endian"},
    {EM_PARISC, "Hewlett-Packard PA-RISC"},
    {EM_VPP500, "Fujitsu VPP500"},
    {EM_SPARC32PLUS, "Enhanced instruction set SPARC"},
    {EM_960, "Intel 80960"},
    {EM_PPC, "PowerPC"},
    {EM_PPC64, "PowerPC64"},
    {EM_S390, "IBM S/390"},
    {EM_SPU, "IBM SPU/SPC"},
    {EM_V800, "NEC V800 series"},
    {EM_FR20, "Fujistsu FR20"},
    {EM_RH32, "TRW RH-32"},
    {EM_RCE, "Motorola RCE"},
    {EM_ARM, "ARM"},
    {EM_ALPHA, "DEC Alpha"},
    {EM_SH, "Hitachi SH"},
    {EM_SPARCV9, "SPARC V9"},
    {EM_TRICORE, "Siemens TriCore"},
    {EM_ARC, "Argonaut RISC Core"},
    {EM_H8_300, "Hitachi H8/300"},
    {EM_H8_300H, "Hitachi H8/300H"},
    {EM_H8S, "Hitachi H8S"},
    {EM_H8_500, "Hitachi H8/500"},
    {EM_IA_64, "Intel IA-64 processor architecture"},
    {EM_MIPS_X, "Stanford MIPS-X"},
    {EM_COLDFIRE, "Motorola Coldfire"},
    {EM_68HC12, "Motorola M68HC12"},
    {EM_MMA, "Fujitsu MMA Multimedia Accelerator"},
    {EM_PCP, "Siemens PCP"},
    {EM_NCPU, "Sony nCPU embedded RISC processor"},
    {EM_NDR1, "Denso NDR1 microprocesspr"},
    {EM_STARCORE, "Motorola Star*Core processor"},
    {EM_ME16, "Toyota ME16 processor"},
    {EM_ST100, "STMicroelectronics ST100 processor"},
    {EM_TINYJ, "Advanced Logic Corp. TinyJ embedded processor family"},
    {EM_X86_64, "AMD x86-64 architecture"},
    {EM_PDSP, "Sony DSP processor"},
    {EM_PDP10, "Digital Equipment Corp. PDP-10"},
    {EM_PDP11, "Digital Equipment Corp. PDP-11"},
    {EM_FX66, "Siemens FX66 microcontroller"},
    {EM_ST9PLUS, "STMicroelectronics ST9+ 8/16 bit microcontroller"},
    {EM_ST7, "STMicroelectronics ST7 8-bit microcontroller"},
    {EM_68HC16, "Motorola MC68HC16 Microcontroller"},
    {EM_68HC11, "Motorola MC68HC11 Microcontroller"},
    {EM_68HC08, "Motorola MC68HC08 Microcontroller"},
    {EM_68HC05, "Motorola MC68HC05 Microcontroller"},
    {EM_SVX, "Silicon Graphics SVx"},
    {EM_ST19, "STMicroelectronics ST19 8-bit microcontroller"},
    {EM_VAX, "Digital VAX"},
    {EM_CRIS, "Axis Communications 32-bit embedded processor"},
    {EM_JAVELIN, "Infineon Technologies 32-bit embedded cpu"},
    {EM_FIREPATH, "Element 14 64-bit DSP processor"},
    {EM_ZSP, "LSI Logic's 16-bit DSP processor"},
    {EM_MMIX, "Donald Knuth's educational 64-bit processor"},
    {EM_HUANY, "Harvard University machine-independent object files"},
    {EM_PRISM, "SiTera Prism"},
    {EM_AVR, "Atmel AVR 8-bit microcontroller"},
    {EM_FR30, "Fujitsu FR30"},
    {EM_D10V, "Mitsubishi D10V"},
    {EM_D30V, "Mitsubishi D30V"},
    {EM_V850, "NEC v850"},
    {EM_M32R, "Renesas M32R (formerly Mitsubishi M32r)"},
    {EM_MN10300, "Matsushita MN10300"},
    {EM_MN10200, "Matsushita MN10200"},
    {EM_PJ, "picoJava"},
    {EM_OPENRISC, "OpenRISC 32-bit embedded processor"},
    {EM_ARC_COMPACT, "ARC International ARCompact processor (old spelling/synonym: EM_ARC_A5)"},
    {EM_XTENSA, "Tensilica Xtensa Processor"},
    {EM_VIDEOCORE, "Alphamosaic VideoCore processor"},
    {EM_TMM_GPP, "Thompson Multimedia General Purpose Processor"},
    {EM_NS32K, "National Semiconductor 32000 series"},
    {EM_TPC, "Tenor Network TPC processor"},
    {EM_SNP1K, "Trebia SNP 1000 processor"},
    {EM_ST200, "STMicroelectronics (www.st.com) ST200"},
    {EM_IP2K, "Ubicom IP2xxx 8-bit microcontrollers"},
    {EM_MAX, "MAX Processor"},
    {EM_CR, "National Semiconductor CompactRISC microprocessor"},
    {EM_F2MC16, "Fujitsu F2MC16"},
    {EM_MSP430, "Texas Instruments embedded microcontroller msp430"},
    {EM_BLACKFIN, "Analog Devices Blackfin (DSP) processor"},
    {EM_SE_C33, "S1C33 Family of Seiko Epson processors"},
    {EM_SEP, "Sharp embedded microprocessor"},
    {EM_ARCA, "Arca RISC microprocessor"},
    {EM_UNICORE, "Microprocessor series from PKU-Unity Ltd. and MPRC"},
    {EM_EXCESS, "eXcess: 16/32/64-bit configurable embedded CPU"},
    {EM_DXP, "Icera Semiconductor Inc. Deep Execution Processor"},
    {EM_ALTERA_NIOS2, "Altera Nios II soft-core processor"},
    {EM_CRX, "National Semiconductor CompactRISC CRX"},
    {EM_XGATE, "Motorola XGATE embedded processor"},
    {EM_C166, "Infineon C16x/XC16x processor"},
    {EM_M16C, "Renesas M16C series microprocessors"},
    {EM_DSPIC30F, "Microchip Technology dsPIC30F Digital Signal"},
    {EM_CE, "Freescale Communication Engine RISC core"},
    {EM_M32C, "Renesas M32C series microprocessors"},
    {EM_TSK3000, "Altium TSK3000 core"},
    {EM_RS08, "Freescale RS08 embedded processor"},
    {EM_SHARC, "Analog Devices SHARC family of 32-bit DSP"},
    {EM_ECOG2, "Cyan Technology eCOG2 microprocessor"},
    {EM_SCORE7, "Sunplus S+core7 RISC processor"},
    {EM_DSP24, "New Japan Radio (NJR) 24-bit DSP Processor"},
    {EM_VIDEOCORE3, "Broadcom VideoCore III processor"},
    {EM_LATTICEMICO32, "RISC processor for Lattice FPGA architecture"},
    {EM_SE_C17, "Seiko Epson C17 family"},
    {EM_TI_C6000, "The Texas Instruments TMS320C6000 DSP family"},
    {EM_TI_C2000, "The Texas Instruments TMS320C2000 DSP family"},
    {EM_TI_C5500, "The Texas Instruments TMS320C55x DSP family"},
    {EM_MMDSP_PLUS, "STMicroelectronics 64bit VLIW Data Signal Processor"},
    {EM_CYPRESS_M8C, "Cypress M8C microprocessor"},
    {EM_R32C, "Renesas R32C series microprocessors"},
    {EM_TRIMEDIA, "NXP Semiconductors TriMedia architecture family"},
    {EM_HEXAGON, "Qualcomm Hexagon processor"},
    {EM_8051, "Intel 8051 and variants"},
    {EM_STXP7X, "STMicroelectronics STxP7x family"},
    {EM_NDS32, "Andes Technology compact code size embedded RISC"},
    {EM_ECOG1, "Cyan Technology eCOG1X family"},
    // FIXME: Following EM_ECOG1X definitions is dead code since EM_ECOG1X has
    //        an identical number to EM_ECOG1.
    {EM_ECOG1X, "Cyan Technology eCOG1X family"},
    {EM_MAXQ30, "Dallas Semiconductor MAXQ30 Core Micro-controllers"},
    {EM_XIMO16, "New Japan Radio (NJR) 16-bit DSP Processor"},
    {EM_MANIK, "M2000 Reconfigurable RISC Microprocessor"},
    {EM_CRAYNV2, "Cray Inc. NV2 vector architecture"},
    {EM_RX, "Renesas RX family"},
    {EM_METAG, "Imagination Technologies META processor"},
    {EM_MCST_ELBRUS, "MCST Elbrus general purpose hardware architecture"},
    {EM_ECOG16, "Cyan Technology eCOG16 family"},
    {EM_CR16, "National Semiconductor CompactRISC CR16 16-bit"},
    {EM_ETPU, "Freescale Extended Time Processing Unit"},
    {EM_SLE9X, "Infineon Technologies SLE9X core"},
    {EM_L10M, "Intel L10M"},
    {EM_K10M, "Intel K10M"},
    {EM_AARCH64, "ARM AArch64"},
    {EM_AVR32, "Atmel Corporation 32-bit microprocessor family"},
    {EM_STM8, "STMicroeletronics STM8 8-bit microcontroller"},
    {EM_TILE64, "Tilera TILE64 multicore architecture family"},
    {EM_TILEPRO, "Tilera TILEPro multicore architecture family"},
    {EM_MICROBLAZE, "Xilinx MicroBlaze 32-bit RISC soft processor core"},
    {EM_CUDA, "NVIDIA CUDA architecture"},
    {EM_TILEGX, "Tilera TILE-Gx multicore architecture family"},
    {EM_CLOUDSHIELD, "CloudShield architecture family"},
    {EM_COREA_1ST, "KIPO-KAIST Core-A 1st generation processor family"},
    {EM_COREA_2ND, "KIPO-KAIST Core-A 2nd generation processor family"},
    {EM_ARC_COMPACT2, "Synopsys ARCompact V2"},
    {EM_OPEN8, "Open8 8-bit RISC soft processor core"},
    {EM_RL78, "Renesas RL78 family"},
    {EM_VIDEOCORE5, "Broadcom VideoCore V processor"},
    {EM_78KOR, "Renesas 78KOR family"},
    {EM_56800EX, "Freescale 56800EX Digital Signal Controller (DSC)"},
    {EM_AMDGPU, "AMD GPU architecture"},
    {EM_RISCV, "RISC-V"},
    {EM_LANAI, "Lanai 32-bit processor"},
    {EM_BPF, "Linux kernel bpf virtual machine"},
    {EM_VE, "NEC SX-Aurora Vector Engine"},
    {EM_CSKY, "C-SKY 32-bit processor"},
    {EM_LOONGARCH, "LoongArch"},
};

} // namespace internal

void writeSymbolsJson(const std::vector<hazel::elf::Symbol> &syms, std::string_view objname, nlohmann::json *j) {
  if (syms.empty()) {
    return;
  }
  auto av = nlohmann::json::array();
  for (const auto &s : syms) {
    av.push_back(nlohmann::json{
        {"name", s.Name},
        {"library", s.Library},
        {"version", s.Version},
        {"vaule", s.Value},
        {"size", s.Size},
        {"sectionindex", s.SectionIndex},
        {"info", static_cast<int>(s.Info)},
        {"other", static_cast<int>(s.Other)},
    });
  }
  j->emplace(objname, std::move(av));
}

void writeSymbolsJson(const std::vector<hazel::elf::ImportedSymbol> &syms, std::string_view objname,
                      nlohmann::json *j) {
  if (syms.empty()) {
    return;
  }
  auto av = nlohmann::json::array();
  for (const auto &s : syms) {
    av.push_back(nlohmann::json{
        {"name", bela::demangle(s.Name)},
        {"library", s.Library},
        {"version", s.Version},
    });
  }
  j->emplace(objname, std::move(av));
}

void writeSymbolsText(const std::vector<hazel::elf::Symbol> &syms, std::string_view objname) {
  if (syms.empty()) {
    return;
  }
  bela::FPrintF(stderr, L"\x1b[34m%s:\x1b[0m\n", objname);
  for (const auto &s : syms) {
    bela::FPrintF(stdout, L"%s(%s@%s)\n", s.Name, s.Library, s.Version);
  }
}

void writeSymbolsText(const std::vector<hazel::elf::ImportedSymbol> &syms, std::string_view objname) {
  if (syms.empty()) {
    return;
  }
  bela::FPrintF(stderr, L"\x1b[34m%s:\x1b[0m\n", objname);
  for (const auto &s : syms) {
    bela::FPrintF(stdout, L"%s (%s@%s)\n", bela::demangle(s.Name), s.Library, s.Version);
  }
}

bool writeSymbols(hazel::elf::File &file, Writer &w) {
  auto j = w.Raw();
  std::vector<hazel::elf::Symbol> syms;
  bela::error_code ec;
  if (file.DynamicSymbols(syms, ec)) {
    (j != nullptr) ? writeSymbolsJson(syms, "dynamicsymbols", j) : writeSymbolsText(syms, "Dynamic Symbols");
  }
  syms.clear();
  if (file.Symbols(syms, ec)) {
    (j != nullptr) ? writeSymbolsJson(syms, "symbols", j) : writeSymbolsText(syms, "Symbols");
  }
  std::vector<hazel::elf::ImportedSymbol> isyms;
  if (file.ImportedSymbols(isyms, ec)) {
    (j != nullptr) ? writeSymbolsJson(syms, "symbols", j) : writeSymbolsText(syms, "Symbols");
  }
  //
  return true;
}

bool AnalysisELF(bela::io::FD &fd, Writer &w) {
  hazel::elf::File file;
  bela::error_code ec;
  if (!file.NewFile(fd.NativeFD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"unable new ELF file %s\n", ec.message);
    return false;
  }
  const auto &fh = file.Fh();
  w.Write(L"Version", static_cast<int>(fh.Version));
  w.Write(L"Machine", internal::stringName(fh.Machine, internal::machines));
  switch (fh.Machine) {
  case hazel::elf::EM_AMDGPU:
    w.Write(L"OSABI", internal::stringName(fh.ABI, internal::amdgpuelfabi));
    break;
  case hazel::elf::EM_TI_C6000:
    w.Write(L"OSABI", internal::stringName(fh.ABI, internal::c6000elfabi));
    break;
  default:
    w.Write(L"OSABI", internal::stringName(fh.ABI, internal::osabiName));
    break;
  }
  w.Write(L"ABIVersion", fh.ABIVersion);
  w.WriteBool(L"Is64Bit", file.Is64Bit());
  w.Write(L"Endian", (fh.Data == hazel::elf::ELFDATA2LSB) ? "LSB" : "MSB");
  w.Write(L"Type", internal::stringName(fh.Type, internal::typeStrings));
  if (auto soname = file.LibSoName(ec); soname) {
    w.Write(L"SONAME", *soname);
  }
  if (auto rpath = file.Rpath(ec); rpath) {
    w.Write(L"RPATH", *rpath);
  }
  if (auto rupath = file.Rpath(ec); rupath) {
    w.Write(L"RUPATH", *rupath);
  }
  std::vector<std::string> libs;
  if (file.Depends(libs, ec)) {
    w.Write(L"Depends", libs);
  }
  if (!IsFullMode) {
    return true;
  }
  writeSymbols(file, w);
  return true;
}

} // namespace bona