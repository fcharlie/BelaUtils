///
#include "bona.hpp"
#include "writer.hpp"
#include <bela/str_cat_narrow.hpp>
#include <hazel/elf.hpp>

namespace bona {
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format

namespace internal {
using namespace hazel::elf;
struct intName {
  int i;
  const char *val;
};

std::string stringNameInternal(uint32_t val, const intName *in, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (in[i].i == val) {
      return in[i].val;
    }
  }
  return std::string(bela::narrow::AlphaNum(val).Piece());
}

template <typename T, size_t N> std::string stringName(T v, const intName (&in)[N]) {
  return stringNameInternal(static_cast<uint32_t>(v), in, N);
}

static constexpr const intName osabiName[] = {
    {ELFOSABI_NONE, "SYSV"},
    {ELFOSABI_HPUX, "HPUX"},
    {ELFOSABI_NETBSD, "NetBSD"},
    {ELFOSABI_LINUX, "Linux"},
    {ELFOSABI_HURD, "Hurd"},
    {5, "ELFOSABI_86OPEN"}, // ELFOSABI_86OPEN
    {ELFOSABI_SOLARIS, "Solaris"},
    {ELFOSABI_AIX, "AIX"},
    {ELFOSABI_IRIX, "IRIX"},
    {ELFOSABI_FREEBSD, "FreeBSD"},
    {ELFOSABI_TRU64, "Tru64"},
    {ELFOSABI_MODESTO, "Novell Modesto"},
    {ELFOSABI_OPENBSD, "OpenBSD"},
    {ELFOSABI_OPENVMS, "OpenVMS"},
    {ELFOSABI_NSK, "Hewlett-Packard Non-Stop Kernel"},
    {ELFOSABI_AROS, "Amiga Research OS"},
    {ELFOSABI_FENIXOS, "FenixOS"},
    {ELFOSABI_CLOUDABI, "CloudABI"},
    {ELFOSABI_AMDGPU_HSA, "AMDGPU OS for HSA"},
    {ELFOSABI_AMDGPU_PAL, "AMDGPU OS for AMD PAL"},
    {ELFOSABI_AMDGPU_MESA3D, "AMDGPU OS for Mesa3D"},
    {ELFOSABI_ARM, "ARM"},
    {255, "ELFOSABI_STANDALONE"},
};

static constexpr const intName typeStrings[] = {
    {0, "NONE"},         {1, "Relocatable file"}, {2, "Executable file"}, {3, "Shared object file"}, {4, "Core file"},
    {0xfe00, "ET_LOOS"}, {0xfeff, "ET_HIOS"},     {0xff00, "ET_LOPROC"},  {0xffff, "ET_HIPROC"},
};

static constexpr const intName machines[] = {
    {0, "EM_NONE"},
    {1, "EM_M32"},
    {2, "EM_SPARC"},
    {3, "EM_386"},
    {4, "EM_68K"},
    {5, "EM_88K"},
    {7, "EM_860"},
    {8, "EM_MIPS"},
    {9, "EM_S370"},
    {10, "EM_MIPS_RS3_LE"},
    {15, "EM_PARISC"},
    {17, "EM_VPP500"},
    {18, "EM_SPARC32PLUS"},
    {19, "EM_960"},
    {20, "EM_PPC"},
    {21, "EM_PPC64"},
    {22, "EM_S390"},
    {36, "EM_V800"},
    {37, "EM_FR20"},
    {38, "EM_RH32"},
    {39, "EM_RCE"},
    {40, "ARM"},
    {42, "EM_SH"},
    {43, "EM_SPARCV9"},
    {44, "EM_TRICORE"},
    {45, "EM_ARC"},
    {46, "EM_H8_300"},
    {47, "EM_H8_300H"},
    {48, "EM_H8S"},
    {49, "EM_H8_500"},
    {50, "EM_IA_64"},
    {51, "EM_MIPS_X"},
    {52, "EM_COLDFIRE"},
    {53, "EM_68HC12"},
    {54, "EM_MMA"},
    {55, "EM_PCP"},
    {56, "EM_NCPU"},
    {57, "EM_NDR1"},
    {58, "EM_STARCORE"},
    {59, "EM_ME16"},
    {60, "EM_ST100"},
    {61, "EM_TINYJ"},
    {62, "x86-64"},
    {63, "EM_PDSP"},
    {64, "EM_PDP10"},
    {65, "EM_PDP11"},
    {66, "EM_FX66"},
    {67, "EM_ST9PLUS"},
    {68, "EM_ST7"},
    {69, "EM_68HC16"},
    {70, "EM_68HC11"},
    {71, "EM_68HC08"},
    {72, "EM_68HC05"},
    {73, "EM_SVX"},
    {74, "EM_ST19"},
    {75, "EM_VAX"},
    {76, "EM_CRIS"},
    {77, "EM_JAVELIN"},
    {78, "EM_FIREPATH"},
    {79, "EM_ZSP"},
    {80, "EM_MMIX"},
    {81, "EM_HUANY"},
    {82, "EM_PRISM"},
    {83, "EM_AVR"},
    {84, "EM_FR30"},
    {85, "EM_D10V"},
    {86, "EM_D30V"},
    {87, "EM_V850"},
    {88, "EM_M32R"},
    {89, "EM_MN10300"},
    {90, "EM_MN10200"},
    {91, "EM_PJ"},
    {92, "EM_OPENRISC"},
    {93, "EM_ARC_COMPACT"},
    {94, "EM_XTENSA"},
    {95, "EM_VIDEOCORE"},
    {96, "EM_TMM_GPP"},
    {97, "EM_NS32K"},
    {98, "EM_TPC"},
    {99, "EM_SNP1K"},
    {100, "EM_ST200"},
    {101, "EM_IP2K"},
    {102, "EM_MAX"},
    {103, "EM_CR"},
    {104, "EM_F2MC16"},
    {105, "EM_MSP430"},
    {106, "EM_BLACKFIN"},
    {107, "EM_SE_C33"},
    {108, "EM_SEP"},
    {109, "EM_ARCA"},
    {110, "EM_UNICORE"},
    {111, "EM_EXCESS"},
    {112, "EM_DXP"},
    {113, "EM_ALTERA_NIOS2"},
    {114, "EM_CRX"},
    {115, "EM_XGATE"},
    {116, "EM_C166"},
    {117, "EM_M16C"},
    {118, "EM_DSPIC30F"},
    {119, "EM_CE"},
    {120, "EM_M32C"},
    {131, "EM_TSK3000"},
    {132, "EM_RS08"},
    {133, "EM_SHARC"},
    {134, "EM_ECOG2"},
    {135, "EM_SCORE7"},
    {136, "EM_DSP24"},
    {137, "EM_VIDEOCORE3"},
    {138, "EM_LATTICEMICO32"},
    {139, "EM_SE_C17"},
    {140, "EM_TI_C6000"},
    {141, "EM_TI_C2000"},
    {142, "EM_TI_C5500"},
    {143, "EM_TI_ARP32"},
    {144, "EM_TI_PRU"},
    {160, "EM_MMDSP_PLUS"},
    {161, "EM_CYPRESS_M8C"},
    {162, "EM_R32C"},
    {163, "EM_TRIMEDIA"},
    {164, "EM_QDSP6"},
    {165, "EM_8051"},
    {166, "EM_STXP7X"},
    {167, "EM_NDS32"},
    {168, "EM_ECOG1"},
    {168, "EM_ECOG1X"},
    {169, "EM_MAXQ30"},
    {170, "EM_XIMO16"},
    {171, "EM_MANIK"},
    {172, "EM_CRAYNV2"},
    {173, "EM_RX"},
    {174, "EM_METAG"},
    {175, "EM_MCST_ELBRUS"},
    {176, "EM_ECOG16"},
    {177, "EM_CR16"},
    {178, "EM_ETPU"},
    {179, "EM_SLE9X"},
    {180, "EM_L10M"},
    {181, "EM_K10M"},
    {183, "Aarch64"},
    {185, "EM_AVR32"},
    {186, "EM_STM8"},
    {187, "EM_TILE64"},
    {188, "EM_TILEPRO"},
    {189, "EM_MICROBLAZE"},
    {190, "EM_CUDA"},
    {191, "EM_TILEGX"},
    {192, "EM_CLOUDSHIELD"},
    {193, "EM_COREA_1ST"},
    {194, "EM_COREA_2ND"},
    {195, "EM_ARC_COMPACT2"},
    {196, "EM_OPEN8"},
    {197, "EM_RL78"},
    {198, "EM_VIDEOCORE5"},
    {199, "EM_78KOR"},
    {200, "EM_56800EX"},
    {201, "EM_BA1"},
    {202, "EM_BA2"},
    {203, "EM_XCORE"},
    {204, "EM_MCHP_PIC"},
    {205, "EM_INTEL205"},
    {206, "EM_INTEL206"},
    {207, "EM_INTEL207"},
    {208, "EM_INTEL208"},
    {209, "EM_INTEL209"},
    {210, "EM_KM32"},
    {211, "EM_KMX32"},
    {212, "EM_KMX16"},
    {213, "EM_KMX8"},
    {214, "EM_KVARC"},
    {215, "EM_CDP"},
    {216, "EM_COGE"},
    {217, "EM_COOL"},
    {218, "EM_NORC"},
    {219, "EM_CSR_KALIMBA "},
    {220, "EM_Z80 "},
    {221, "EM_VISIUM "},
    {222, "EM_FT32 "},
    {223, "EM_MOXIE"},
    {224, "EM_AMDGPU"},
    {243, "EM_RISCV"},
    {244, "EM_LANAI"},
    {247, "EM_BPF"},

    /* Non-standard or deprecated. */
    {6, "EM_486"},
    {10, "EM_MIPS_RS4_BE"},
    {41, "EM_ALPHA_STD"},
    {0x9026, "EM_ALPHA"},
};

} // namespace internal

bool AnalysisELF(bela::File &fd, Writer &w) {
  hazel::elf::File file;
  bela::error_code ec;
  if (!file.NewFile(fd.FD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"unable new ELF file %s\n", ec.message);
    return false;
  }
  const auto &fh = file.Fh();
  w.Write(L"Version", static_cast<int>(fh.Version));
  w.Write(L"Machine", internal::stringName(fh.Machine, internal::machines));
  w.WriteBool(L"Is64Bit", file.Is64Bit());
  w.Write(L"Endian", (fh.Data == hazel::elf::ELFDATA2LSB) ? "LSB" : "MSB");
  w.Write(L"Type", internal::stringName(fh.Type, internal::typeStrings));
  w.Write(L"OSABI", internal::stringName(fh.ABI, internal::osabiName));
  w.Write(L"ABIVersion", fh.ABIVersion);
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
  return true;
}

} // namespace bona