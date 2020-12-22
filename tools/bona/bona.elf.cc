///
#include "bona.hpp"
#include "writer.hpp"
#include <hazel/elf.hpp>

namespace bona {
const wchar_t *elf_osabi(uint8_t osabi) {
  using namespace hazel::elf;
  switch (osabi) {
  case ELFOSABI_NONE:
    return L"SYSV";
  case ELFOSABI_HPUX:
    return L"HP-UX";
  case ELFOSABI_NETBSD:
    return L"NetBSD";
  case ELFOSABI_LINUX:
    return L"Linux";
  case ELFOSABI_HURD: /// musl not defined
    return L"Hurd";
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
  case ELFOSABI_CLOUDABI:
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

bool AnalysisELF(bela::File &fd, Writer &w) {
  hazel::elf::File file;
  bela::error_code ec;
  if (!file.NewFile(fd.FD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"unable new ELF file %s\n", ec.message);
    return false;
  }
  w.Write(L"OSABI", elf_osabi(file.Fh().OSABI));
  if (auto soname = file.LibSoName(ec); soname) {
    w.Write(L"SoName", *soname);
  }
  //
  return true;
}

} // namespace bona