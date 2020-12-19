///
#ifndef BELAVIEW_HPP
#define BELAVIEW_HPP
#include <bela/terminal.hpp>
#include <bela/pe.hpp>
#include <bela/time.hpp>
#include <bela/datetime.hpp>
#include <hazel/hazel.hpp>
#include <hazel/elf.hpp>
#include <hazel/macho.hpp>
#include <json.hpp>

namespace belaview {
inline void RecordErrorCode(nlohmann::json *j, std::wstring_view file, const bela::error_code &ec) {
  if (j = nullptr) {
    return;
  }
  try {
    nlohmann::json j2;
    j2["error_code"] = ec.code;
    j2["file"] = bela::ToNarrow(file);
    j2["message"] = bela::ToNarrow(ec.message);
    j->push_back(std::move(j2));
  } catch (const std::exception &) {
  }
}

bool ViewPE(bela::File &fd, nlohmann::json *j);
bool ViewZIP(bela::File &fd, nlohmann::json *j);
bool ViewELF(bela::File &fd, nlohmann::json *j);
bool ViewMachO(bela::File &fd, nlohmann::json *j);

} // namespace belaview

#endif