/// https://lowlevelbits.org/parsing-mach-o-files/
#include "hazelinc.hpp"
#include "macho.hpp"

// Some docs
// 1. Mach-O loader for Linux
// https://github.com/shinh/maloader
//
// 2. PARSING MACH-O FILES
// https://lowlevelbits.org/parsing-mach-o-files/

namespace hazel::internal {
class macho_memview {
public:
private:
};

std::optional<macho_particulars_result> explore_macho(std::wstring_view sv, bela::error_code &ec) {
  //
  return std::nullopt;
}

} // namespace hazel::internal

// No more plans to parse Mach-O file details