//
#include <bela/codecvt.hpp>
#include <compact_enc_det/compact_enc_det.h>
namespace bona {
std::wstring FileNameRecoding(std::string_view name) {
  bool is_reliable = false;
  int bytes_consumed = 0;
  auto enc = CompactEncDet::DetectEncoding(name.data(), static_cast<int>(name.size()), nullptr, nullptr, nullptr,
                                           UNKNOWN_ENCODING, UNKNOWN_LANGUAGE, CompactEncDet::WEB_CORPUS, false,
                                           &bytes_consumed, &is_reliable);
  switch (enc) {
  case UTF8:
    break;
  case CHINESE_GB:
    break;
  }
  return L"";
}
} // namespace bona