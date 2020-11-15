///
#include "hazelinc.hpp"
#include <bela/endian.hpp>

namespace hazel::internal {
#pragma pack(2)
struct zip_file_header_t {
  uint8_t magic[4]; //{'P','K','',''} // 0x04034b50 LE
  uint16_t version;
  uint16_t bitflag;
  uint16_t method;
  uint16_t mtime;
  uint16_t mdate;
  uint32_t crc32;
  uint32_t compressedsize;
  uint32_t uncompressedsize;
  uint16_t namelen;
  uint16_t fieldlength;
};

struct zip_file_info64_t {
  uint16_t version;
  uint16_t versionneeded;
  uint16_t flag;
  uint16_t method;
  uint32_t dosdate; // uint61_t + uint64_t
  uint32_t crc;
  uint64_t compressedsize;
  uint64_t uncompressedsize;
  uint16_t sizefilename;
  uint16_t sizefileextra;
  uint16_t sizefilecomment;
  uint16_t disknumstart;
  uint16_t internalfa;
  uint16_t externalfa;
};

#pragma pack()

inline bool IsZip(const byte_t *buf, size_t size) {
  return (size > 3 && buf[0] == 0x50 && buf[1] == 0x4B && (buf[2] == 0x3 || buf[2] == 0x5 || buf[2] == 0x7) &&
          (buf[3] == 0x4 || buf[3] == 0x6 || buf[3] == 0x8));
}

status_t msdocssubview(bela::MemView mv, particulars_result &pr) {
  if (mv.StartsWith("word/")) {
    pr.assign(L"Microsoft Word (.docx)", types::docx);
    return Found;
  }
  if (mv.StartsWith("ppt/")) {
    pr.assign(L"Microsoft PowerPoint (.pptx)", types::pptx);
    return Found;
  }
  if (mv.StartsWith("xl/")) {
    pr.assign(L"Microsoft Excel (.xlsx)", types::xlsx);
    return Found;
  }
  return None;
}

void *Memmem(const void *haystack, size_t haystack_len, const void *needle, size_t needle_len) {
  const char *begin = reinterpret_cast<const char *>(haystack);
  const char *last_possible = begin + haystack_len - needle_len;
  const char *tail = reinterpret_cast<const char *>(needle);
  char point;

  /*
   * The first occurrence of the empty string is deemed to occur at
   * the beginning of the string.
   */
  if (needle_len == 0)
    return (void *)begin;

  /*
   * Sanity check, otherwise the loop might search through the whole
   * memory.
   */
  if (haystack_len < needle_len)
    return nullptr;

  point = *tail++;
  for (; begin <= last_possible; begin++) {
    if (*begin == point && !memcmp(begin + 1, tail, needle_len - 1))
      return (void *)begin;
  }
  return nullptr;
}

using ssize_t = SSIZE_T;

ssize_t MagicIndex(bela::MemView mv, size_t offset) {
  constexpr const byte_t docsMagic[] = {'P', 'K', 0x03, 0x04};
  if (offset > mv.size()) {
    return -1;
  }
  auto p = Memmem(mv.data() + offset, mv.size() - offset, docsMagic, ArrayLength(docsMagic));
  if (p == nullptr) {
    return -1;
  }
  return reinterpret_cast<const uint8_t *>(p) - mv.data();
}

status_t inquisitive_msxmldocs(bela::MemView mv, particulars_result &pr) {
  constexpr const byte_t docsMagic[] = {'P', 'K', 0x03, 0x04};
  if (!mv.StartsWith(docsMagic)) {
    return None;
  }
  if (msdocssubview(mv.submv(0x1E), pr) == Found) {
    return Found;
  }
  /// SO we known. first file must [Content_Types.xml]
  if (!mv.IndexsWith(0x1E, "[Content_Types].xml") && !mv.IndexsWith(0x1E, "_rels/.rels")) {
    return None;
  }
  auto hd = mv.cast<zip_file_header_t>(0);
  if (hd == nullptr) {
    return None;
  }
  ssize_t startoffset = bela::readle<uint32_t>(mv.data() + 18) + 49;
  auto index = MagicIndex(mv, startoffset);
  if (index == -1) {
    return None;
  }
  startoffset = index + 4 + 26;
  index = MagicIndex(mv, startoffset);
  if (index == -1) {
    return None;
  }
  startoffset = index + 4 + 26;
  if (msdocssubview(mv.submv(startoffset), pr) == Found) {
    return Found;
  }
  startoffset += 26;
  index = MagicIndex(mv, startoffset);
  if (index == -1) {
    return None;
  }
  startoffset = index + 4 + 26;
  if (msdocssubview(mv.submv(startoffset), pr) == Found) {
    return Found;
  }
  // auto offset=
  return None;
}

status_t explore_zip_family(bela::MemView mv, particulars_result &pr) {
  if (!IsZip(mv.data(), mv.size())) {
    return None;
  }
  if (inquisitive_msxmldocs(mv, pr) == Found) {
    return Found;
  }
  pr.assign(L"Zip archive data", types::zip);
  return Found;
}
} // namespace hazel::internal