/// ELF details
#include "elf.h"
#include "hazelinc.hpp"
#include <bela/codecvt.hpp>
#include <bela/strcat.hpp>
#include <bela/endian.hpp>

//  Executable and Linkable Format ELF
// Thanks musl libc
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// http://hg.icculus.org/icculus/fatelf/raw-file/tip/docs/fatelf-specification.txt

namespace hazel {

namespace internal {

const wchar_t *elf_osabi(uint8_t osabi) {
  switch (osabi) {
  case ELFOSABI_SYSV:
    return L"SYSV";
  case ELFOSABI_HPUX:
    return L"HP-UX";
  case ELFOSABI_NETBSD:
    return L"NetBSD";
  case ELFOSABI_LINUX:
    return L"Linux";
  case 4: /// musl not defined
    return L"GNU Hurd";
  case ELFOSABI_SOLARIS:
    return L"Solaris";
  case ELFOSABI_AIX:
    return L"AIX";
  case ELFOSABI_IRIX:
    return L"IRIX";
  case ELFOSABI_FREEBSD:
    return L"FreeBSD";
  case ELFOSABI_TRU64:
    return L"Tru64";
  case ELFOSABI_MODESTO:
    return L"Novell Modesto";
  case ELFOSABI_OPENBSD:
    return L"OpenBSD";
  case ELFOSABI_OPENVMS:
    return L"OpenVMS";
  case ELFOSABI_NSK:
    return L"Hewlett-Packard Non-Stop Kernel";
  case ELFOSABI_AROS:
    return L"Amiga Research OS";
  case ELFOSABI_FENIXOS:
    return L"FenixOS";
  case 0x11:
    return L"CloudABI";
  case ELFOSABI_AMDGPU_HSA:
    return L"AMDGPU OS for HSA";
  case ELFOSABI_AMDGPU_PAL:
    return L"AMDGPU OS for AMD PAL";
  case ELFOSABI_AMDGPU_MESA3D:
    return L"AMDGPU OS for Mesa3D ";
  case ELFOSABI_ARM:
    return L"ARM";
  default:
    break;
  }
  return L"UNKNOWN";
}
struct elf_kv_t {
  uint32_t index;
  const wchar_t *value;
};

const wchar_t *elf_machine(uint32_t e) {
  const elf_kv_t kv[] = {
      {EM_M32, L"AT&T WE 32100"},
      {EM_SPARC, L"SUN SPARC"},
      {EM_386, L"x86"},
      {EM_68K, L"Motorola m68k family"},
      {EM_88K, L"Motorola m88k family"},
      {EM_860, L"Intel 80860"},
      {EM_MIPS, L"MIPS R3000 (officially, big-endian only)"},
      {EM_S370, L"IBM System/370"},
      {EM_MIPS_RS3_LE, L"MIPS R3000 little-endian (Oct 4 1999 Draft) Deprecated"},
      {EM_PARISC, L"HPPA"},
      {EM_VPP500, L"Fujitsu VPP500"},
      {EM_SPARC32PLUS, L"Sun's v8plus"},
      {EM_960, L"Intel 80960"},
      {EM_PPC, L"PowerPC"},
      {EM_PPC64, L"64-bit PowerPC"},
      {EM_S390, L"IBM System/390"},
      {EM_SPU, L"Sony/Toshiba/IBM SPU"},
      {EM_V800, L"NEC V800 series"},
      {EM_FR20, L"Fujitsu FR20"},
      {EM_RH32, L"TRW RH32"},
      {EM_RCE, L"Motorola M*Core // May also be taken by Fujitsu MMA"},
      {EM_ARM, L"ARM"},
      {EM_FAKE_ALPHA, L"Digital Alpha"},
      {EM_SH, L"Renesas (formerly Hitachi) / SuperH SH"},
      {EM_SPARCV9, L"SPARC v9 64-bit"},
      {EM_TRICORE, L"Siemens Tricore embedded processor"},
      {EM_ARC, L"ARC Cores"},
      {EM_H8_300, L"Renesas (formerly Hitachi) H8/300"},
      {EM_H8_300H, L"Renesas (formerly Hitachi) H8/300H"},
      {EM_H8S, L"Renesas (formerly Hitachi) H8S"},
      {EM_H8_500, L"Renesas (formerly Hitachi) H8/500"},
      {EM_IA_64, L"Intel IA-64 Processor"},
      {EM_MIPS_X, L"Stanford MIPS-X"},
      {EM_COLDFIRE, L"Motorola Coldfire"},
      {EM_68HC12, L"Motorola M68HC12"},
      {EM_MMA, L"Fujitsu Multimedia Accelerator"},
      {EM_PCP, L"Siemens PCP"},
      {EM_NCPU, L"Sony nCPU embedded RISC processor"},
      {EM_NDR1, L"Denso NDR1 microprocesspr"},
      {EM_STARCORE, L"Motorola Star*Core processor"},
      {EM_ME16, L"Toyota ME16 processor"},
      {EM_ST100, L"STMicroelectronics ST100 processor"},
      {EM_TINYJ, L"Advanced Logic Corp. TinyJ embedded processor"},
      {EM_X86_64, L"x86-64"},
      {EM_PDSP, L"Sony DSP Processor"},
      {EM_PDP10, L"Digital Equipment Corp. PDP-10"},
      {EM_PDP11, L"Digital Equipment Corp. PDP-11"},
      {EM_FX66, L"Siemens FX66 microcontroller"},
      {EM_ST9PLUS, L"STMicroelectronics ST9+ 8/16 bit microcontroller"},
      {EM_ST7, L"STMicroelectronics ST7 8-bit microcontroller"},
      {EM_68HC16, L"Motorola MC68HC16 Microcontroller"},
      {EM_68HC11, L"Motorola MC68HC11 Microcontroller"},
      {EM_68HC08, L"Motorola MC68HC08 Microcontroller"},
      {EM_68HC05, L"Motorola MC68HC05 Microcontroller"},
      {EM_SVX, L"Silicon Graphics SVx"},
      {EM_ST19, L"STMicroelectronics ST19 8-bit cpu"},
      {EM_VAX, L"Digital VAX"},
      {EM_CRIS, L"Axis Communications 32-bit embedded processor"},
      {EM_JAVELIN, L"Infineon Technologies 32-bit embedded cpu"},
      {EM_FIREPATH, L"Element 14 64-bit DSP processor"},
      {EM_ZSP, L"LSI Logic's 16-bit DSP processor"},
      {EM_MMIX, L"Donald Knuth's educational 64-bit processor"},
      {EM_HUANY, L"Harvard's machine-independent format"},
      {EM_PRISM, L"SiTera Prism"},
      {EM_AVR, L"Atmel AVR 8-bit microcontroller"},
      {EM_FR30, L"Fujitsu FR30"},
      {EM_D10V, L"Mitsubishi D10V"},
      {EM_D30V, L"Mitsubishi D30V"},
      {EM_V850, L"NEC v850"},
      {EM_M32R, L"Renesas M32R (formerly Mitsubishi M32R)"},
      {EM_MN10300, L"Matsushita MN10300"},
      {EM_MN10200, L"Matsushita MN10200"},
      {EM_PJ, L"picoJava"},
      {EM_OPENRISC, L"OpenRISC 32-bit embedded processor"},
      {EM_ARC_A5, L"ARC Cores Tangent-A5"},
      {EM_XTENSA, L"Tensilica Xtensa Architecture"},
      {EM_VIDEOCORE, L"Alphamosaic VideoCore processor"},
      {EM_TMM_GPP, L"Thompson Multimedia General Purpose Processor"},
      {EM_NS32K, L"National Semiconductor 32000 series"},
      {EM_TPC, L"Tenor Network TPC processor"},
      {EM_SNP1K, L"Trebia SNP 1000 processor"},
      {EM_ST200, L"STMicroelectronics ST200 microcontroller"},
      {EM_IP2K, L"Ubicom IP2022 micro controller"},
      {EM_MAX, L"MAX Processor"},
      {EM_CR, L"National Semiconductor CompactRISC"},
      {EM_F2MC16, L"Fujitsu F2MC16"},
      {EM_MSP430, L"TI msp430 micro controller"},
      {EM_BLACKFIN, L"ADI Blackfin"},
      {EM_SE_C33, L"S1C33 Family of Seiko Epson processors"},
      {EM_SEP, L"Sharp embedded microprocessor"},
      {EM_ARCA, L"Arca RISC Microprocessor"},
      {EM_UNICORE, L"Microprocessor series from PKU-Unity Ltd. and MPRC of "
                   L"Peking University"},
      {EM_EXCESS, L"eXcess: 16/32/64-bit configurable embedded CPU"},
      {EM_DXP, L"Icera Semiconductor Inc. Deep Execution Processor"},
      {EM_ALTERA_NIOS2, L"Altera Nios II soft-core processor"},
      {EM_CRX, L"National Semiconductor CRX"},
      {EM_XGATE, L"Motorola XGATE embedded processor"},
      {EM_C166, L"Infineon C16x/XC16x processor"},
      {EM_M16C, L"Renesas M16C series microprocessors"},
      {EM_DSPIC30F, L"Microchip Technology dsPIC30F Digital Signal Controller"},
      {EM_CE, L"Freescale Communication Engine RISC core"},
      {EM_M32C, L"Renesas M32C series microprocessors"},
      {EM_TSK3000, L"Altium TSK3000 core"},
      {EM_RS08, L"Freescale RS08 embedded processor"},
      {EM_SHARC, L"SHARC"},
      {EM_ECOG2, L"Cyan Technology eCOG2 microprocessor"},
      {EM_SCORE7, L"Sunplus S+core7 RISC processor"},
      {EM_DSP24, L"New Japan Radio (NJR) 24-bit DSP Processor"},
      {EM_VIDEOCORE3, L"Broadcom VideoCore III processor"},
      {EM_LATTICEMICO32, L"RISC processor for Lattice FPGA architecture"},
      {EM_SE_C17, L"Seiko Epson C17 family"},
      {EM_TI_C6000, L"Texas Instruments TMS320C6000 DSP family"},
      {EM_TI_C2000, L"Texas Instruments TMS320C2000 DSP family"},
      {EM_TI_C5500, L"Texas Instruments TMS320C55x DSP family"},
      {EM_TI_ARP32, L"Texas Instruments  ARP32"},
      {EM_TI_PRU, L"Texas Instruments  PRU"},
      {EM_MMDSP_PLUS, L"STMicroelectronics 64bit VLIW Data Signal Processor"},
      {EM_CYPRESS_M8C, L"Cypress M8C microprocessor"},
      {EM_R32C, L"Renesas R32C series microprocessors"},
      {EM_TRIMEDIA, L"NXP Semiconductors TriMedia architecture family"},
      {EM_QDSP6, L"QUALCOMM DSP6 Processor"},
      {EM_8051, L"Intel 8051 and variants"},
      {EM_STXP7X, L"STMicroelectronics STxP7x family"},
      {EM_NDS32, L"Andes Technology compact code size embedded RISC processor family"},
      {EM_ECOG1X, L"Cyan Technology eCOG1X family"},
      {EM_MAXQ30, L"Dallas Semiconductor MAXQ30 Core Micro-controllers"},
      {EM_XIMO16, L"New Japan Radio (NJR) 16-bit DSP Processor"},
      {EM_MANIK, L"M2000 Reconfigurable RISC Microprocessor"},
      {EM_CRAYNV2, L"Cray Inc. NV2 vector architecture"},
      {EM_RX, L"Renesas RX family"},
      {EM_METAG, L"Imagination Technologies META processor architecture"},
      {EM_MCST_ELBRUS, L"MCST Elbrus general purpose hardware architecture"},
      {EM_ECOG16, L"Cyan Technology eCOG16 family"},
      {EM_CR16, L"National Semiconductor CompactRISC 16-bit processor"},
      {EM_ETPU, L"Freescale Extended Time Processing Unit"},
      {EM_SLE9X, L"Infineon Technologies SLE9X core"},
      {EM_L10M, L"Intel L1OM"},
      {EM_K10M, L"Intel K1OM"},
      {EM_AARCH64, L"AArch64"},
      {EM_AVR32, L"Atmel Corporation 32-bit microprocessor family"},
      {EM_STM8, L"STMicroeletronics STM8 8-bit microcontroller"},
      {EM_TILE64, L"Tilera TILE64 multicore architecture family"},
      {EM_TILEPRO, L"Tilera TILEPro multicore architecture family"},
      {EM_MICROBLAZE, L"Xilinx MicroBlaze 32-bit RISC soft processor core"},
      {EM_CUDA, L"NVIDIA CUDA architecture"},
      {EM_TILEGX, L"Tilera TILE-Gx multicore architecture family"},
      {EM_CLOUDSHIELD, L"CloudShield architecture family"},
      {EM_COREA_1ST, L"KIPO-KAIST Core-A 1st generation processor family"},
      {EM_COREA_2ND, L"KIPO-KAIST Core-A 2nd generation processor family"},
      {EM_ARC_COMPACT2, L"Synopsys ARCompact V2"},
      {EM_OPEN8, L"Open8 8-bit RISC soft processor core"},
      {EM_RL78, L"Renesas RL78 family"},
      {EM_VIDEOCORE5, L"Broadcom VideoCore V processor"},
      {EM_78KOR, L"Renesas 78KOR family"},
      {EM_56800EX, L"Freescale 56800EX Digital Signal Controller (DSC)"},
      {EM_BA1, L"Beyond BA1 CPU architecture"},
      {EM_BA2, L"Beyond BA2 CPU architecture"},
      {EM_XCORE, L"XMOS xCORE processor family"},
      {EM_MCHP_PIC, L"Microchip 8-bit PIC(r) family"},
      {EM_KM32, L"KM211 KM32 32-bit processor"},
      {EM_KMX32, L"KM211 KMX32 32-bit processor"},
      {EM_EMX16, L"KM211 KMX16 16-bit processor"},
      {EM_EMX8, L"KM211 KMX8 8-bit processor"},
      {EM_KVARC, L"KM211 KVARC processor"},
      {EM_CDP, L"Paneve CDP architecture family"},
      {EM_COGE, L"Cognitive Smart Memory Processor"},
      {EM_COOL, L"iCelero CoolEngine"},
      {EM_NORC, L"Nanoradio Optimized RISC"},
      {EM_CSR_KALIMBA, L"CSR Kalimba architecture family"},
      {EM_Z80, L"Zilog Z80"},
      {EM_VISIUM, L"Controls and Data Services VISIUMcore processor"},
      {EM_FT32, L"FTDI Chip FT32 high performance 32-bit RISC architecture"},
      {EM_MOXIE, L"Moxie processor family"},
      {EM_AMDGPU, L"AMD GPU architecture"},
      {EM_RISCV, L"RISC-V"},
      {EM_LANAI, L"Lanai processor"},
      {EM_CEVA, L"CEVA Processor Architecture Family"},
      {EM_CEVA_X2, L"CEVA X2 Processor Family"},
      {EM_BPF, L"BPF"},
      {EM_NUM, L"NUM"},
      {EM_ALPHA, L"EM_ALPHA"}
      ///
  };
  for (const auto &k : kv) {
    if (k.index == e) {
      return k.value;
    }
  }
  return L"No specific instruction set";
}

const wchar_t *elf_object_type(uint16_t t) {
  switch (t) {
  case ET_NONE:
    return L"No file type";
  case ET_REL:
    return L"Relocatable file ";
  case ET_EXEC:
    return L"Executable file";
  case ET_DYN:
    return L"Shared object file";
  case ET_CORE:
    return L"Core file";
  }
  return L"UNKNOWN";
}

inline endian::endian_t Endian(uint8_t t) {
  switch (t) {
  case ELFDATANONE:
    return endian::None;
  case ELFDATA2LSB:
    return endian::LittleEndian;
  case ELFDATA2MSB:
    return endian::BigEndian;
  default:
    break;
  }
  return endian::None;
}

class elf_memview {
public:
  elf_memview(bela::MemView mv) : data_(reinterpret_cast<const char *>(mv.data())), size_(mv.size()) {
    //
  }
  const char *data() const { return data_; }
  size_t size() const { return size_; }
  template <typename T> const T *cast(size_t off) const {
    if (off + sizeof(T) >= size_) {
      return nullptr;
    }
    return reinterpret_cast<const T *>(data_ + off);
  }
  template <typename Integer> Integer resive(Integer i) {
    if (!resiveable) {
      return i;
    }
    return bela::bswap(i);
  }
  std::string stroffset(size_t off, size_t end);
  bool inquisitive(elf_particulars_result &em, bela::error_code &ec);
  bool inquisitive64(elf_particulars_result &em, bela::error_code &ec);

private:
  const char *data_{nullptr};
  size_t size_{0};
  bool resiveable{false};
};

std::string elf_memview::stroffset(size_t off, size_t end) {
  std::string s;
  for (size_t i = off; i < end; i++) {
    if (data_[i] == 0) {
      break;
    }
    s.push_back(data_[i]);
  }
  return s;
}

//
bool elf_memview::inquisitive64(elf_particulars_result &em, bela::error_code &ec) {
  auto h = cast<Elf64_Ehdr>(0);
  if (h == nullptr) {
    return false;
  }
  em.machine = elf_machine(resive(h->e_machine));
  em.etype = elf_object_type(resive(h->e_type));
  auto off = resive(h->e_shoff);
  auto sects = cast<Elf64_Shdr>(off);
  auto shnum = resive(h->e_shnum);
  if (shnum * sizeof(Elf64_Shdr) + off > size_) {
    ec = bela::make_error_code(1, L"ELF file size too small");
    return false;
  }
  Elf64_Off sh_offset = 0;
  Elf64_Xword sh_entsize = 0;
  Elf64_Xword sh_size = 0;
  Elf64_Word sh_link = 0;
  for (Elf64_Word i = 0; i < shnum; i++) {
    auto st = resive(sects[i].sh_type);
    if (st == SHT_DYNAMIC) {
      sh_entsize = resive(sects[i].sh_entsize);
      sh_offset = resive(sects[i].sh_offset);
      sh_size = resive(sects[i].sh_size);
      sh_link = resive(sects[i].sh_link);
      continue;
    }
  }

  if (sh_offset == 0 || sh_entsize == 0 || sh_offset >= size_) {
    return true;
  }
  auto strtab = &sects[sh_link];
  if (sh_link >= shnum) {
    ec = bela::make_error_code(1, L"ELF file size too small");
    return false;
  }

  Elf64_Off soff = resive(strtab->sh_offset);
  Elf64_Off send = soff + resive(strtab->sh_size);
  auto n = sh_size / sh_entsize;
  auto dyn = cast<Elf64_Dyn>(sh_offset);
  for (decltype(n) i = 0; i < n; i++) {
    auto first = resive(dyn[i].d_un.d_val);
    switch (resive(dyn[i].d_tag)) {
    case DT_NEEDED: {
      auto deps = stroffset(soff + first, send);
      em.depends.push_back(bela::ToWide(deps));
    } break;
    case DT_SONAME:
      em.soname = bela::ToWide(stroffset(soff + first, send));
      break;
    case DT_RUNPATH:
      em.rupath = bela::ToWide(stroffset(soff + first, send));
      break;
    case DT_RPATH:
      em.rpath = bela::ToWide(stroffset(soff + first, send));
      break;
    default:
      break;
    }
  }

  return true;
}

bool elf_memview::inquisitive(elf_particulars_result &em, bela::error_code &ec) {
  em.endian = Endian(static_cast<uint8_t>(data_[EI_DATA]));
  em.osabi = elf_osabi(data_[EI_OSABI]);
  em.version = data_[EI_VERSION];
  auto msb = (em.endian == endian::BigEndian);
  resiveable = (msb != bela::IsBigEndianHost);
  int eic = data_[EI_CLASS];
  if (eic == ELFCLASS64) {
    em.bit64 = true;
    return inquisitive64(em, ec);
  }
  if (eic != ELFCLASS32) {
    ec = bela::make_error_code(1, L"EI_CLASS invalid:", eic);
    return false;
  }
  auto h = cast<Elf32_Ehdr>(0);
  em.machine = elf_machine(resive(h->e_machine));
  em.etype = elf_object_type(resive(h->e_type));
  auto off = resive(h->e_shoff);
  auto sects = cast<Elf32_Shdr>(off);
  auto shnum = resive(h->e_shnum);
  if (shnum * sizeof(Elf32_Shdr) + off > size_) {
    ec = bela::make_error_code(1, L"ELF file size too small");
    return false;
  }
  Elf32_Off sh_offset = 0;
  Elf32_Xword sh_entsize = 0;
  Elf32_Xword sh_size = 0;
  Elf32_Word sh_link = 0;
  for (Elf32_Word i = 0; i < shnum; i++) {
    auto st = resive(sects[i].sh_type);
    if (st == SHT_DYNAMIC) {
      sh_entsize = resive(sects[i].sh_entsize);
      sh_offset = resive(sects[i].sh_offset);
      sh_size = resive(sects[i].sh_size);
      sh_link = resive(sects[i].sh_link);
      continue;
    }
  }

  if (sh_offset == 0 || sh_entsize == 0 || sh_offset >= size_) {
    return true;
  }
  auto strtab = &sects[sh_link];
  if (sh_link >= shnum) {
    return false;
  }

  Elf32_Off soff = resive(strtab->sh_offset);
  Elf32_Off send = soff + resive(strtab->sh_size);
  auto n = sh_size / sh_entsize;
  auto dyn = cast<Elf32_Dyn>(sh_offset);

  for (decltype(n) i = 0; i < n; i++) {
    auto first = resive(dyn[i].d_un.d_val);
    switch (resive(dyn[i].d_tag)) {
    case DT_NEEDED: {
      auto deps = stroffset(soff + first, send);
      em.depends.push_back(bela::ToWide(deps));
    } break;
    case DT_SONAME:
      em.soname = bela::ToWide(stroffset(soff + first, send));
      break;
    case DT_RUNPATH:
      em.rupath = bela::ToWide(stroffset(soff + first, send));
      break;
    case DT_RPATH:
      em.rpath = bela::ToWide(stroffset(soff + first, send));
      break;
    default:
      break;
    }
  }
  return true;
}
} // namespace internal
std::optional<elf_particulars_result> explore_elf(std::wstring_view sv, bela::error_code &ec) {
  bela::MapView mv;
  if (!mv.MappingView(sv, ec, sizeof(Elf32_Ehdr))) {
    return std::nullopt;
  }
  internal::elf_memview emv(mv.subview());
  elf_particulars_result em;
  if (emv.inquisitive(em, ec)) {
    return std::make_optional<elf_particulars_result>(std::move(em));
  }
  return std::nullopt;
}

} // namespace hazel
