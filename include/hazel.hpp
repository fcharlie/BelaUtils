//
#ifndef HAZEL_HPP
#define HAZEL_HPP
#include <bela/base.hpp>
#include <bela/codecvt.hpp>
#include <optional>
// include types
#include "details/hazeltypes.hpp"

namespace hazel {
extern bool IsDebugMode;
struct hazel_attribute_t {
  std::wstring name;
  std::wstring value;
  hazel_attribute_t() = default;
  hazel_attribute_t(const std::wstring_view n, const std::wstring_view v) : name(n), value(v) {}
  hazel_attribute_t(std::wstring &&n, std::wstring &&v) : name(std::move(n)), value(std::move(v)) {}
};
struct hazel_multi_attribute_t {
  std::wstring name;
  std::vector<std::wstring> values;
  hazel_multi_attribute_t() = default;
  hazel_multi_attribute_t(const std::wstring_view n, const std::vector<std::wstring> &v) : name(n), values(v) {}
  hazel_multi_attribute_t(std::wstring &&n, std::vector<std::wstring> &&v) : name(std::move(n)), values(std::move(v)) {}
};

class particulars_result {
public:
  static constexpr size_t deslen = sizeof("Description") - 1;
  particulars_result() = default;
  particulars_result(std::wstring_view n, types::catalog_t ct = types::catalog_t::none) { assign(n, ct); }
  particulars_result(particulars_result &&other) { assign(std::move(other)); }
  particulars_result(const particulars_result &other) { assign(other); }
  particulars_result &operator=(particulars_result &&other) { return assign(std::move(other)); }
  particulars_result &operator=(const particulars_result &other) { return assign(other); }
  particulars_result &assign(std::wstring_view n, types::catalog_t ct = types::catalog_t::none) {
    name = n;
    t = ct;
    return *this;
  }
  particulars_result &assign(particulars_result &&other) {
    name.assign(std::move(other.name));
    t = other.t;
    alignlen = other.alignlen;
    attrs = std::move(other.attrs);
    mattrs = std::move(other.mattrs);
    return *this;
  }
  particulars_result &assign(const particulars_result &other) {
    name = other.name;
    t = other.t;
    alignlen = other.alignlen;
    attrs = other.attrs;
    mattrs = other.mattrs;
    return *this;
  }
  particulars_result &add(std::wstring_view name, std::wstring_view value) {
    alignlen = (std::max)(alignlen, bela::StringWidth(name) + 2);
    attrs.emplace_back(name, value);
    return *this;
  }
  particulars_result &add(std::wstring_view name, std::vector<std::wstring> &&value) {
    alignlen = (std::max)(alignlen, bela::StringWidth(name) + 2);
    mattrs.emplace_back(name, std::move(value));
    return *this;
  }
  template <typename... Args>
  particulars_result &add(std::wstring_view name, std::wstring_view value, std::wstring_view attr1, Args... attrN) {
    alignlen = (std::max)(alignlen, bela::StringWidth(name) + 2);
    std::vector<std::wstring> av{value, attr1, attrN...};
    mattrs.emplace_back(name, std::move(av));
    return *this;
  }
  bool same_elf() const {
    return t == types::elf || t == types::elf_executable || t == types::elf_relocatable ||
           t == types::elf_shared_object;
  }
  bool same_macho() const {
    return t == types::macho_bundle || t == types::macho_core || t == types::macho_dsym_companion ||
           t == types::macho_dynamic_linker || t == types::macho_dynamically_linked_shared_lib ||
           t == types::macho_dynamically_linked_shared_lib_stub || t == types::macho_executable ||
           t == types::macho_fixed_virtual_memory_shared_lib || t == types::macho_kext_bundle ||
           t == types::macho_object || t == types::macho_preload_executable || t == types::macho_universal_binary;
  }
  bool same_pecoff() const { return t == types::pecoff_executable; }
  // return
  const auto &attributes() const { return attrs; }
  const auto &multi_attributes() const { return mattrs; }
  std::wstring_view class_name() const { return name; }
  auto catalog() const { return t; }
  auto align_length() const { return alignlen; }
  void clear() {
    name.clear();
    attrs.clear();
    mattrs.clear();
    alignlen = deslen;
    t = types::catalog_t::none;
  }

private:
  std::wstring name;
  std::vector<hazel_attribute_t> attrs;
  std::vector<hazel_multi_attribute_t> mattrs;
  std::size_t alignlen{deslen}; // description
  types::catalog_t t{types::catalog_t::none};
};

std::optional<particulars_result> explore_file(std::wstring_view file, bela::error_code &ec);

namespace endian {
enum endian_t : unsigned { None, LittleEndian, BigEndian };
}
struct elf_particulars_result {
  std::wstring machine;
  std::wstring osabi;
  std::wstring etype;
  std::wstring rpath;                // RPATH or some
  std::wstring rupath;               // RUPATH
  std::wstring soname;               // SONAME
  std::vector<std::wstring> depends; /// require so
  std::vector<hazel_attribute_t> attributes;
  std::vector<hazel_multi_attribute_t> multi_attributes;
  int version;
  endian::endian_t endian;
  bool bit64{false}; /// 64 Bit
  elf_particulars_result &add(std::wstring_view name, std::wstring_view value) {
    attributes.emplace_back(name, value);
    return *this;
  }
  elf_particulars_result &add(std::wstring_view name, std::vector<std::wstring> &&value) {
    multi_attributes.emplace_back(name, std::move(value));
    return *this;
  }
  template <typename... Args>
  elf_particulars_result &add(std::wstring_view name, std::wstring_view value, std::wstring_view attr1, Args... attrN) {
    std::vector<std::wstring> av{value, attr1, attrN...};
    multi_attributes.emplace_back(name, std::move(av));
    return *this;
  }
};

struct pe_version_t {
  uint16_t major{0};
  uint16_t minor{0};
};

struct pe_particulars_result {
  std::wstring machine;
  std::wstring subsystem;
  std::wstring clrmsg;
  std::vector<std::wstring> characteristics;
  std::vector<std::wstring> depends; /// DLL required
  std::vector<std::wstring> delays;  //
  std::vector<hazel_attribute_t> attributes;
  std::vector<hazel_multi_attribute_t> multi_attributes;
  pe_version_t osver;
  pe_version_t linkver;
  pe_version_t imagever;
  bool isdll;
  pe_particulars_result &add(std::wstring_view name, std::wstring_view value) {
    attributes.emplace_back(name, value);
    return *this;
  }
  pe_particulars_result &add(std::wstring_view name, std::vector<std::wstring> &&value) {
    multi_attributes.emplace_back(name, std::move(value));
    return *this;
  }
  template <typename... Args>
  pe_particulars_result &add(std::wstring_view name, std::wstring_view value, std::wstring_view attr1, Args... attrN) {
    std::vector<std::wstring> av{value, attr1, attrN...};
    multi_attributes.emplace_back(name, std::move(av));
    return *this;
  }
};

struct macho_particulars_result {
  std::wstring machine;
  std::wstring mtype; /// Mach-O type
  std::vector<hazel_attribute_t> attributes;
  std::vector<hazel_multi_attribute_t> multi_attributes;
  bool isfat{false};
  bool is64abi{false};
  macho_particulars_result &add(std::wstring_view name, std::wstring_view value) {
    attributes.emplace_back(name, value);
    return *this;
  }
  macho_particulars_result &add(std::wstring_view name, std::vector<std::wstring> &&value) {
    multi_attributes.emplace_back(name, std::move(value));
    return *this;
  }
  template <typename... Args>
  macho_particulars_result &add(std::wstring_view name, std::wstring_view value, std::wstring_view attr1,
                                Args... attrN) {
    std::vector<std::wstring> av{value, attr1, attrN...};
    multi_attributes.emplace_back(name, std::move(av));
    return *this;
  }
};
/// PE/ELF these files need to be parsed in depth, perhaps to read more bytes of files.
std::optional<pe_particulars_result> explore_pecoff(std::wstring_view sv, bela::error_code &ec);
std::optional<elf_particulars_result> explore_elf(std::wstring_view sv, bela::error_code &ec);
} // namespace hazel

#endif