///
#include <hazel/zip.hpp>
#include "bona.hpp"
#include "writer.hpp"

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

namespace bona {
// TODO Filename encoding convert
// https://mimesniff.spec.whatwg.org/
bool AnalysisZipFamily(const hazel::zip::Reader &r, Writer &w) {
  switch (r.LooksLikeMsZipContainer()) {
  case hazel::zip::OfficeDocx:
    w.Write(L"Extension", L"Microsoft Word 2007+");
    w.Write(L"MIME", L"application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    return true;
  case hazel::zip::OfficePptx:
    w.Write(L"Extension", L"Microsoft PowerPoint 2007+");
    w.Write(L"MIME", L"application/vnd.openxmlformats-officedocument.presentationml.presentation");
    return true;
  case hazel::zip::OfficeXlsx:
    w.Write(L"Extension", L"Microsoft Excel 2007+");
    w.Write(L"MIME", L"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    return true;
  case hazel::zip::NuGetPackage:
    w.Write(L"Extension", L"NuGet Package");
    w.Write(L"MIME", L"application/octet-stream");
    return true;
  default:
    break;
  }
  if (r.LooksLikeAppx()) {
    w.Write(L"Extension", L"Windows App Package");
    w.Write(L"MIME", L"application/vnd.microsoft.package-archive");
    return true;
  }
  if (r.LooksLikeApk()) {
    w.Write(L"Extension", L"Android APK");
    w.Write(L"MIME", L"application/vnd.android.package-archive");
    return true;
  }
  if (r.LooksLikeOFD()) {
    w.Write(L"Extension", L"Open Fixed-layout Document");
    w.Write(L"Standard", L"GB/T 33190-2016");
    w.Write(L"MIME", L"application/vnd.open-fixed-layout-document");
    return true;
  }
  if (r.LooksLikeJar()) {
    w.Write(L"Extension", L"Java Archive");
    w.Write(L"MIME", L"application/java-archive");
    return true;
  }
  std::string odfmime;
  if (r.LooksLikeODF(&odfmime)) {
    w.Write(L"Extension", L"OpenDocument Format");
    w.Write(L"MIME", odfmime);
    return true;
  }
  return false;
}

void ZipAssginMethods(const hazel::zip::Reader &r, Writer &w) {
  std::vector<uint16_t> methods;
  for (const auto &f : r.Files()) {
    if (std::find(methods.begin(), methods.end(), f.method) == methods.end()) {
      methods.emplace_back(f.method);
    }
  }
  std::vector<std::wstring> ms;
  for (const auto m : methods) {
    ms.emplace_back(hazel::zip::Method(m));
  }
  w.Write(L"Method", ms);
}

bool AssginFiles(const hazel::zip::Reader &r, nlohmann::json *j) {
  auto fv = nlohmann::json::array();
  for (const auto &file : r.Files()) {
    if (file.IsEncrypted()) {
      fv.push_back(nlohmann::json{
          {"name", file.name},
          {"method", file.method},
          {"uncompressedsize", file.uncompressedSize},
          {"compressedsize", file.compressedSize},
          {"version", file.rversion},
          {"position", file.position},
          {"flags", file.flags},
          {"comment", file.comment},
          {"crc32", file.crc32},
          {"time", bela::FormatUniversalTimeNarrow(file.time)},
          {"attrs", file.externalAttrs},
          {"aesversion", file.aesVersion},
      });
      continue;
    }
    fv.push_back(nlohmann::json{
        {"name", file.name},
        {"method", file.method},
        {"uncompressedsize", file.uncompressedSize},
        {"compressedsize", file.compressedSize},
        {"version", file.rversion},
        {"position", file.position},
        {"flags", file.flags},
        {"comment", file.comment},
        {"crc32", file.crc32},
        {"time", bela::FormatUniversalTimeNarrow(file.time)},
        {"attrs", file.externalAttrs},
    });
  }
  j->emplace("files", std::move(fv));
  return true;
}

enum FileMode : uint32_t {
  ModeDir = 2147483648,      // d: is a directory
  ModeAppend = 1073741824,   // a: append-only
  ModeExclusive = 536870912, // l: exclusive use
  ModeTemporary = 268435456, // T: temporary file; Plan 9 only
  ModeSymlink = 134217728,   // L: symbolic link
  ModeDevice = 67108864,     // D: device file
  ModeNamedPipe = 33554432,  // p: named pipe (FIFO)
  ModeSocket = 16777216,     // S: Unix domain socket
  ModeSetuid = 8388608,      // u: setuid
  ModeSetgid = 4194304,      // g: setgid
  ModeCharDevice = 4194304,  // c: Unix character device, when ModeDevice is set
  ModeSticky = 1048576,      // t: sticky
  ModeIrregular = 524288,    // ?: non-regular file; nothing else is known about this file

  // Mask for the type bits. For regular files, none will be set.
  ModeType = ModeDir | ModeSymlink | ModeNamedPipe | ModeSocket | ModeDevice | ModeCharDevice | ModeIrregular,
};
[[nodiscard]] constexpr FileMode operator&(FileMode L, FileMode R) noexcept {
  using I = std::underlying_type_t<FileMode>;
  return static_cast<FileMode>(static_cast<I>(L) & static_cast<I>(R));
}
[[nodiscard]] constexpr FileMode operator|(FileMode L, FileMode R) noexcept {
  using I = std::underlying_type_t<FileMode>;
  return static_cast<FileMode>(static_cast<I>(L) | static_cast<I>(R));
}

constexpr auto s_IFMT = 0xf000;
constexpr auto s_IFSOCK = 0xc000;
constexpr auto s_IFLNK = 0xa000;
constexpr auto s_IFREG = 0x8000;
constexpr auto s_IFBLK = 0x6000;
constexpr auto s_IFDIR = 0x4000;
constexpr auto s_IFCHR = 0x2000;
constexpr auto s_IFIFO = 0x1000;
constexpr auto s_ISUID = 0x800;
constexpr auto s_ISGID = 0x400;
constexpr auto s_ISVTX = 0x200;

constexpr auto msdosDir = 0x10;
constexpr auto msdosReadOnly = 0x01;

std::string FileModeToString(FileMode m) {
  constexpr const char str[] = "dalTLDpSugct?";
  char buf[32] = {0}; //
  int w = 0;
  for (int i = 0; i < 13; i++) {
    if ((m & (1 << (32 - 1 - i))) != 0) {
      buf[w] = str[i];
      w++;
    }
  }
  if (w == 0) {
    buf[w] = '-';
    w++;
  }
  constexpr const char rwx[] = "rwxrwxrwx";
  for (int i = 0; i < 11; i++) {
    if ((m & (1 << (9 - 1 - i))) != 0) {
      buf[w] = rwx[i];
    } else {
      buf[w] = '-';
    }
    w++;
  }
  return std::string(buf, w);
}

FileMode unixModeToFileMode(uint32_t m) {
  uint32_t mode = static_cast<FileMode>(m & 0777);
  switch (m & s_IFMT) {
  case s_IFBLK:
    mode |= ModeDevice;
    break;
  case s_IFCHR:
    mode |= ModeDevice | ModeCharDevice;
    break;
  case s_IFDIR:
    mode |= ModeDir;
    break;
  case s_IFIFO:
    mode |= ModeNamedPipe;
    break;
  case s_IFLNK:
    mode |= ModeSymlink;
    break;
  case s_IFREG:
    break;
  case s_IFSOCK:
    mode |= ModeSocket;
  default:
    break;
  }
  if ((m & s_ISGID) != 0) {
    mode |= ModeSetgid;
  }
  if ((m & s_ISUID) != 0) {
    mode |= ModeSetuid;
  }
  if ((m & s_ISVTX) != 0) {
    mode |= ModeSticky;
  }
  return static_cast<FileMode>(mode);
}

FileMode msdosModeToFileMode(uint32_t m) {
  uint32_t mode = 0;
  if ((m & msdosDir) != 0) {
    mode = ModeDir | 0777;
  } else {
    mode = 0666;
  }
  if ((m & msdosReadOnly) != 0) {
    mode &= ~0222;
  }
  return static_cast<FileMode>(mode);
}

constexpr auto creatorFAT = 0;
constexpr auto creatorUnix = 3;
constexpr auto creatorNTFS = 11;
constexpr auto creatorVFAT = 14;
constexpr auto creatorMacOSX = 19;

std::string ZipModeString(const hazel::zip::File &file) {
  FileMode mode = static_cast<FileMode>(0);
  auto n = file.cversion >> 8;
  if (n == creatorUnix || n == creatorMacOSX) {
    mode = unixModeToFileMode(file.externalAttrs >> 16);
  } else if (n == creatorNTFS || n == creatorVFAT || n == creatorFAT) {
    mode = msdosModeToFileMode(file.externalAttrs);
  }
  if (file.name.ends_with('/')) {
    mode = mode | ModeDir;
  }
  return FileModeToString(mode);
}

bool AnalysisZIP(bela::File &fd, Writer &w) {
  hazel::zip::Reader r;
  bela::error_code ec;
  if (!r.OpenReader(fd.FD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"ZIP OpenReader: %s\n", ec.message);
    return false;
  }
  AnalysisZipFamily(r, w);
  w.Write(L"Counts", r.Files().size());
  w.Write(L"USize", r.UncompressedSize());
  w.Write(L"CSize", r.CompressedSize());
  if (!r.Comment().empty()) {
    w.Write(L"Comments", r.Comment());
  }
  if (!IsFullMode) {
    ZipAssginMethods(r, w);
    return true;
  }
  if (auto j = w.Raw(); j != nullptr) {
    AssginFiles(r, j);
    return true;
  }
  // TODO fix zip ls
  for (const auto &file : r.Files()) {
    bela::FPrintF(stdout, L"%s\t%s\t%d\t%s\t%s\n", ZipModeString(file), hazel::zip::Method(file.method),
                  file.uncompressedSize, bela::FormatTime(file.time), file.name);
  }
  return true;
}

} // namespace bona