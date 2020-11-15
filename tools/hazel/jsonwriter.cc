///
#include <hazel.hpp>
#include <json.hpp>
#include <bela/codecvt.hpp>
#include <bela/terminal.hpp>
#include <bela/narrow/strcat.hpp>

inline nlohmann::json json_array(const std::vector<std::wstring> &wv) {
  auto jv = nlohmann::json::array();
  for (const auto &v : wv) {
    jv.emplace_back(bela::ToNarrow(v));
  }
  return jv;
}

bool WriteToJson(FILE *fd, const hazel::particulars_result &pr) {
  try {
    nlohmann::json j;
    j["description"] = bela::ToNarrow(pr.description());
    j["mime"] = bela::ToNarrow(pr.mime());
    for (const auto &it : pr.attributes()) {
      j[bela::ToNarrow(it.first)] = bela::ToNarrow(it.second);
    }
    for (const auto &it : pr.multi_attributes()) {
      j[bela::ToNarrow(it.first)] = json_array(it.second);
    }
    bela::terminal::WriteAuto(fd, j.dump(4));
  } catch (const std::exception &) {
    return false;
  }
  return true;
}

inline std::string make_version(hazel::pe_version_t pv) { return bela::narrow::StringCat(pv.major, ".", pv.minor); }

bool WriteToJson(FILE *fd, const hazel::pe_particulars_result &pr) {
  try {
    nlohmann::json j;
    j["description"] = "PE executable file";
    j["mime"] = "application/vnd.microsoft.portable-executable";
    j["subsystem"] = bela::ToNarrow(pr.subsystem);
    j["machine"] = bela::ToNarrow(pr.machine);
    j["characteristics"] = json_array(pr.characteristics);
    j["depends"] = json_array(pr.depends);
    j["delay_depends"] = json_array(pr.delays);
    j["is_dll"] = pr.isdll;
    j["os_version"] = make_version(pr.osver);
    j["linker_version"] = make_version(pr.linkver);
    j["image_version"] = make_version(pr.imagever);
    for (const auto &it : pr.attributes) {
      j[bela::ToNarrow(it.first)] = bela::ToNarrow(it.second);
    }
    for (const auto &it : pr.multi_attributes) {
      j[bela::ToNarrow(it.first)] = json_array(it.second);
    }
    bela::terminal::WriteAuto(fd, j.dump(4));
  } catch (const std::exception &) {
    return false;
  }
  return true;
}

bool WriteToJson(FILE *fd, const hazel::elf_particulars_result &pr) {
  try {
    nlohmann::json j;
    j["description"] = "ELF executable binary";
    j["mime"] = "application/x-executable";
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
    j["depends"] = json_array(pr.depends);
    for (const auto &it : pr.attributes) {
      j[bela::ToNarrow(it.first)] = bela::ToNarrow(it.second);
    }
    for (const auto &it : pr.multi_attributes) {
      j[bela::ToNarrow(it.first)] = json_array(it.second);
    }
    bela::terminal::WriteAuto(fd, j.dump(4));
  } catch (const std::exception &) {
    return false;
  }
  return true;
}

bool WriteToJson(FILE *fd, const hazel::macho_particulars_result &pr) {
  try {
    nlohmann::json j;
    j["description"] = "Mach-O Executable binary";
    j["mime"] = "application/x-mach-binary";
    for (const auto &it : pr.attributes) {
      j[bela::ToNarrow(it.first)] = bela::ToNarrow(it.second);
    }
    for (const auto &it : pr.multi_attributes) {
      j[bela::ToNarrow(it.first)] = json_array(it.second);
    }
    bela::terminal::WriteAuto(fd, j.dump(4));
  } catch (const std::exception &) {
    return false;
  }
  return true;
}
