///
#include <hazel.hpp>
#include <json.hpp>
#include <bela/codecvt.hpp>
#include <bela/terminal.hpp>
#include <bela/narrow/strcat.hpp>

inline std::vector<std::string> NarrowArray(const std::vector<std::wstring> &wv) {
  std::vector<std::string> av;
  for (const auto &v : wv) {
    av.emplace_back(bela::ToNarrow(v));
  }
  return av;
}

bool WriteToJson(FILE *fd, const hazel::particulars_result &pr) {
  try {
    nlohmann::json j;
    j["description"] = bela::ToNarrow(pr.description());
    j["mime"] = bela::ToNarrow(pr.mime());
    for (const auto &it : pr.attributes()) {
      j[bela::ToNarrow(it.name)] = bela::ToNarrow(it.value);
    }
    for (const auto &it : pr.multi_attributes()) {
      j[bela::ToNarrow(it.name)] = NarrowArray(it.values);
    }
    auto body = j.dump(4);
    fwrite(body.data(), 1, body.size(), fd);
  } catch (const std::exception &) {
    return false;
  }
  return true;
}

inline std::string make_version(hazel::pe_version_t pv) { return bela::narrow::StringCat(pv.major, ".", pv.minor); }

bool WriteToJson(FILE *fd, const hazel::pe_particulars_result &pr) {
  try {
    nlohmann::json j;
    j["description"] = L"PE executable file";
    j["mime"] = L"application/vnd.microsoft.portable-executable";
    j["subsystem"] = bela::ToNarrow(pr.subsystem);
    j["machine"] = bela::ToNarrow(pr.machine);
    j["characteristics"] = NarrowArray(pr.characteristics);
    j["depends"] = NarrowArray(pr.depends);
    j["delay_depends"] = NarrowArray(pr.delays);
    j["is_dll"] = pr.isdll;
    j["os_version"] = make_version(pr.osver);
    j["linker_version"] = make_version(pr.linkver);
    j["image_version"] = make_version(pr.imagever);
    for (const auto &it : pr.attributes) {
      j[bela::ToNarrow(it.name)] = bela::ToNarrow(it.value);
    }
    for (const auto &it : pr.multi_attributes) {
      j[bela::ToNarrow(it.name)] = NarrowArray(it.values);
    }
    auto body = j.dump(4);
    fwrite(body.data(), 1, body.size(), fd);
  } catch (const std::exception &) {
    return false;
  }
  return true;
}

bool WriteToJson(FILE *fd, const hazel::elf_particulars_result &pr) {
  try {
    nlohmann::json j;
    j["description"] = L"ELF executable binary";
    j["mime"] = L"application/x-executable";
    j["machine"] = bela::ToNarrow(pr.machine);
    j["abi"] = bela::ToNarrow(pr.osabi);
    j["elf_type"] = bela::ToNarrow(pr.etype);
    if (!pr.rupath.empty()) {
      j["runpath"] = bela::ToNarrow(pr.rupath);
    }
    if (!pr.rpath.empty()) {
      j["rpath"] = bela::ToNarrow(pr.rpath);
    }
    if (!pr.soname.empty()) {
      j["soname"] = bela::ToNarrow(pr.soname);
    }
    j["64bit"] = pr.bit64;
    if (pr.endian == hazel::endian::BigEndian) {
      j["endian"] = "BigEndian";
    } else {
      j["endian"] = "LittleEndian";
    }
    j["depends"] = NarrowArray(pr.depends);
    for (const auto &it : pr.attributes) {
      j[bela::ToNarrow(it.name)] = bela::ToNarrow(it.value);
    }
    for (const auto &it : pr.multi_attributes) {
      j[bela::ToNarrow(it.name)] = NarrowArray(it.values);
    }
    auto body = j.dump(4);
    fwrite(body.data(), 1, body.size(), fd);
  } catch (const std::exception &) {
    return false;
  }
  return true;
}

bool WriteToJson(FILE *fd, const hazel::macho_particulars_result &pr) {
  try {
    nlohmann::json j;
    j["description"] = L"Mach-O Executable binary";
    j["mime"] = L"application/x-mach-binary";
    for (const auto &it : pr.attributes) {
      j[bela::ToNarrow(it.name)] = bela::ToNarrow(it.value);
    }
    for (const auto &it : pr.multi_attributes) {
      j[bela::ToNarrow(it.name)] = NarrowArray(it.values);
    }
    auto body = j.dump(4);
    fwrite(body.data(), 1, body.size(), fd);
  } catch (const std::exception &) {
    return false;
  }
  return true;
}
