///
#include "kisasumutils.hpp"
#include <charconv>
#include <bela/base.hpp>
#include <bela/match.hpp>
#include <bela/strip.hpp>
#include <bela/path.hpp>
#include <bela/codecvt.hpp>
#include <json.hpp>

namespace kisasum {

class FileAtomicWriter {
public:
  FileAtomicWriter(std::wstring_view p) : path(p) { locktmp = bela::StringCat(path, L".locktmp"); }
  FileAtomicWriter(const FileAtomicWriter &) = delete;
  FileAtomicWriter &operator=(const FileAtomicWriter &) = delete;
  ~FileAtomicWriter() {
    if (fd != nullptr) {
      fclose(fd);
      DeleteFileW(locktmp.data());
    }
  }
  bool Open();
  bool Flush();
  FILE *FileHandle() const { return fd; }

private:
  std::wstring path;
  std::wstring locktmp;
  FILE *fd{nullptr};
};

bool FileAtomicWriter::Open() {
  if (fd != nullptr) {
    return false;
  }
  if (_wfopen_s(&fd, locktmp.data(), L"w+") != 0) {
    return false;
  }
  return true;
}

bool FileAtomicWriter::Flush() {
  if (!bela::PathExists(path)) {
    fclose(fd);
    fd = nullptr;
    return MoveFileW(locktmp.data(), path.data()) == TRUE;
  }
  auto oldtmp = bela::StringCat(path, L".oldtmp");
  if (bela::PathExists(oldtmp) && (DeleteFileW(oldtmp.data()) != TRUE)) {
    return false;
  }
  if (MoveFileW(path.data(), oldtmp.data()) != TRUE) {
    return false;
  }
  if (MoveFileW(locktmp.data(), path.data()) != TRUE) {
    MoveFileW(oldtmp.data(), path.data());
    return false;
  }
  DeleteFileW(oldtmp.data());
  return true;
}

// RGB(1,2,3)
bool RgbColorExpand(std::string_view scr, std::uint32_t &cr) {
  if (!kisasum::ConsumePrefix(&scr, "RGB(") || !kisasum::ConsumeSuffix(&scr, ")")) {
    return false;
  }
  uint32_t r = 0;
  uint32_t g = 0;
  uint32_t b = 0;
  auto end = scr.data() + scr.size();
  auto begin = scr.data();
  auto ec = std::from_chars(begin, end, r);
  if (ec.ec != std::errc{} || ec.ptr + 1 >= end) {
    return false;
  }
  begin = ec.ptr + 1;
  ec = std::from_chars(begin, end, g);
  if (ec.ec != std::errc{} || ec.ptr + 1 >= end) {
    return false;
  }
  begin = ec.ptr + 1;
  ec = std::from_chars(begin, end, b);
  if (ec.ec != std::errc{} || ec.ptr + 1 >= end) {
    return false;
  }
  return true;
}

bool InitializeColorValue(std::string_view scr, std::uint32_t &cr) {
  if (scr.size() < 4) {
    return false;
  }
  if (scr.front() == '#') {
    scr.remove_prefix(1);
    auto ec = std::from_chars(scr.data(), scr.data() + scr.size(), cr, 16);
    if (ec.ec != std::errc{}) {
      return false;
    }
    return true;
  }
  return RgbColorExpand(scr, cr);
}

template <size_t N> std::string_view EncodeColor(std::uint32_t cr, char (&buf)[N]) {
  auto k = _snprintf_s(buf, N, "#%06X", cr);
  return std::string_view(buf, k);
}

inline std::wstring KisasumOptionsFile() {
  auto dwlen = GetModuleFileNameW(nullptr, nullptr, 0);
  if (dwlen == 0) {
    return false;
  }
  std::wstring str;
  str.resize(dwlen);
  dwlen = GetModuleFileNameW(nullptr, str.data(), dwlen);
  str.resize(dwlen);
  return bela::StringCat(str, L"../kisasum.json");
}

bool InitializeKisasumOptions(KisasumOptions &options) {
  auto jsonfile = KisasumOptionsFile();
  FILE *fd = nullptr;
  if (_wfopen_s(&fd, jsonfile.data(), L"rb") != 0) {
    return false;
  }
  try {
    auto json = nlohmann::json::parse(fd);
    if (auto it = json.find("Title"); it != json.end()) {
      options.title = bela::ToWide(it->get<std::string_view>());
    }
    if (auto it = json.find("Font"); it != json.end()) {
      options.font = bela::ToWide(it->get<std::string_view>());
    }
    if (auto it = json.find("Color.Panel"); it != json.end()) {
      InitializeColorValue(it->get<std::string_view>(), options.panelcolor);
    }
    if (auto it = json.find("Color.Content"); it != json.end()) {
      InitializeColorValue(it->get<std::string_view>(), options.contentcolor);
    }
    if (auto it = json.find("Color.Text"); it != json.end()) {
      InitializeColorValue(it->get<std::string_view>(), options.textcolor);
    }
    if (auto it = json.find("Color.Label"); it != json.end()) {
      InitializeColorValue(it->get<std::string_view>(), options.labelcolor);
    }
  } catch (const std::exception &) {
    fclose(fd);
    return false;
  }
  fclose(fd);
  return true;
}

bool FlushKisasumOptions(const KisasumOptions &options) {
  // we flush options only panel color
  KisasumOptions no;
  InitializeKisasumOptions(no);
  no.panelcolor = options.panelcolor;
  try {
    nlohmann::json j;
    char buf[16];
    j["Title"] = bela::ToNarrow(no.title);
    j["Font"] = bela::ToNarrow(no.font);
    j["Color.Panel"] = EncodeColor(options.panelcolor, buf);
    j["Color.Content"] = EncodeColor(options.contentcolor, buf);
    j["Color.Text"] = EncodeColor(options.textcolor, buf);
    j["Color.Label"] = EncodeColor(options.labelcolor, buf);
    auto tojson = j.dump(4);
    auto jsonfile = KisasumOptionsFile();
    FileAtomicWriter fw(jsonfile);
    if (!fw.Open()) {
      return false;
    }
    if (fwrite(tojson.data(), 1, tojson.size(), fw.FileHandle()) != tojson.size()) {
      return false;
    }
    fw.Flush();
  } catch (const std::exception &) {
    return false;
  }
  return true;
}

} // namespace kisasum