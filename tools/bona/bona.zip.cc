///
#include <hazel/zip.hpp>
#include "bona.hpp"
#include "writer.hpp"

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

namespace bona {

/*
 * legal utf-8 byte sequence
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - page 94
 *
 *  Code Points        1st       2s       3s       4s
 * U+0000..U+007F     00..7F
 * U+0080..U+07FF     C2..DF   80..BF
 * U+0800..U+0FFF     E0       A0..BF   80..BF
 * U+1000..U+CFFF     E1..EC   80..BF   80..BF
 * U+D000..U+D7FF     ED       80..9F   80..BF
 * U+E000..U+FFFF     EE..EF   80..BF   80..BF
 * U+10000..U+3FFFF   F0       90..BF   80..BF   80..BF
 * U+40000..U+FFFFF   F1..F3   80..BF   80..BF   80..BF
 * U+100000..U+10FFFF F4       80..8F   80..BF   80..BF
 *
 */

// Thanks
// https://github.com/lemire/Code-used-on-Daniel-Lemire-s-blog/blob/master/2018/05/08/checkutf8.c
static const uint8_t utf8d[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,        // 00..1f
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,        // 20..3f
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,        // 40..5f
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   //
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,        // 60..7f
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   //
    1,   1,   1,   1,   1,   9,   9,   9,   9,   9,   9,   //
    9,   9,   9,   9,   9,   9,   9,   9,   9,   9,        // 80..9f
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   //
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   //
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,        // a0..bf
    8,   8,   2,   2,   2,   2,   2,   2,   2,   2,   2,   //
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   //
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,        // c0..df
    0xa, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, //
    0x3, 0x3, 0x4, 0x3, 0x3,                               // e0..ef
    0xb, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, //
    0x8, 0x8, 0x8, 0x8, 0x8                                // f0..ff
};

static const uint8_t utf8d_transition[] = {
    0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4, //
    0x6, 0x1, 0x1, 0x1, 0x1,                               // s0..s0
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   //
    1,   1,   1,   1,   1,   1,   0,   1,   1,   1,   1,   //
    1,   0,   1,   0,   1,   1,   1,   1,   1,   1,        // s1..s2
    1,   2,   1,   1,   1,   1,   1,   2,   1,   2,   1,   //
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   //
    1,   2,   1,   1,   1,   1,   1,   1,   1,   1,        // s3..s4
    1,   2,   1,   1,   1,   1,   1,   1,   1,   2,   1,   //
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   //
    1,   3,   1,   3,   1,   1,   1,   1,   1,   1,        // s5..s6
    1,   3,   1,   1,   1,   1,   1,   3,   1,   3,   1,   //
    1,   1,   1,   1,   1,   1,   3,   1,   1,   1,   1,   //
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,        // s7..s8
};

static inline uint32_t updatestate(uint32_t *state, uint32_t byte) {
  uint32_t type = utf8d[byte];
  *state = utf8d_transition[16 * *state + type];
  return *state;
}

bool validate_utf8(const char *c, size_t len) {
  const unsigned char *cu = (const unsigned char *)c;
  uint32_t state = 0;
  for (size_t i = 0; i < len; i++) {
    uint32_t byteval = (uint32_t)cu[i];
    if (updatestate(&state, byteval) == UTF8_REJECT) {
      return false;
    }
  }
  return true;
}
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

bool AnalysisZIP(bela::File &fd, Writer &w) {
  hazel::zip::Reader r;
  bela::error_code ec;
  if (!r.OpenReader(fd.FD(), bela::SizeUnInitialized, ec)) {
    w.WriteError(ec);
    bela::FPrintF(stderr, L"ZIP OpenReader: %s\n", ec.message);
    return false;
  }
  AnalysisZipFamily(r, w);
  w.Write(L"Files", r.Files().size());
  w.Write(L"USize", r.UncompressedSize());
  w.Write(L"CSize", r.CompressedSize());
  if (!r.Comment().empty()) {
    w.Write(L"Comments", r.Comment());
  }
  if (!IsFullMode) {
    ZipAssginMethods(r, w);
    return true;
  }
  return true;
}

} // namespace bona