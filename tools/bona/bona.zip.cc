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
  w.Write(L"MIME", L"application/zip");
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
    if (file.IsFileNameUTF8()) {
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
            {"crc32", file.crc32sum},
            {"time", bela::FormatUniversalTimeNarrow(file.time)},
            {"mode", hazel::zip::String(file.mode)},
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
          {"crc32", file.crc32sum},
          {"time", bela::FormatUniversalTimeNarrow(file.time)},
          {"mode", hazel::zip::String(file.mode)},
      });
      continue;
    }
    auto name = bela::ToNarrow(FileNameRecoding(file.name));
    if (file.IsEncrypted()) {
      fv.push_back(nlohmann::json{
          {"name", name},
          {"method", file.method},
          {"uncompressedsize", file.uncompressedSize},
          {"compressedsize", file.compressedSize},
          {"version", file.rversion},
          {"position", file.position},
          {"flags", file.flags},
          {"comment", file.comment},
          {"crc32", file.crc32sum},
          {"time", bela::FormatUniversalTimeNarrow(file.time)},
          {"mode", hazel::zip::String(file.mode)},
          {"aesversion", file.aesVersion},
      });
      continue;
    }
    fv.push_back(nlohmann::json{
        {"name", name},
        {"method", file.method},
        {"uncompressedsize", file.uncompressedSize},
        {"compressedsize", file.compressedSize},
        {"version", file.rversion},
        {"position", file.position},
        {"flags", file.flags},
        {"comment", file.comment},
        {"crc32", file.crc32sum},
        {"time", bela::FormatUniversalTimeNarrow(file.time)},
        {"mode", hazel::zip::String(file.mode)},
    });
  }
  j->emplace("files", std::move(fv));
  return true;
}

bool AnalysisZIP(bela::io::FD &fd, Writer &w) {
  hazel::zip::Reader r;
  bela::error_code ec;
  if (!r.OpenReader(fd.NativeFD(), bela::SizeUnInitialized, ec)) {
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
    // good UTF-8
    if (file.IsFileNameUTF8()) {
      bela::FPrintF(stdout, L"%s\t%s\t%d\t%s\t%s\n", hazel::zip::String(file.mode), hazel::zip::Method(file.method),
                    file.uncompressedSize, bela::FormatTime(file.time), file.name);
      continue;
    }
    bela::FPrintF(stdout, L"%s\t%s\t%d\t%s\t%s(*)\n", hazel::zip::String(file.mode), hazel::zip::Method(file.method),
                  file.uncompressedSize, bela::FormatTime(file.time), FileNameRecoding(file.name));
  }
  return true;
}

} // namespace bona