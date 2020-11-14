///
#include "hazelinc.hpp"

namespace hazel {
bool IsDebugMode = false;


std::optional<particulars_result> explore_file(std::wstring_view file, bela::error_code &ec) {
  bela::MapView mmv;
  if (!mmv.MappingView(file, ec, 1, 32 * 1024)) {
    return std::nullopt;
  }
  auto mv = mmv.subview();
  particulars_result pr;
  // if (auto found = internal::explore_binobj(mv, pr); found == internal::Found) {
  //   bela::error_code ec2;
  //   if (!explore_executable(mv, pr, ec2)) {
  //     DbgPrint(L"resolve executable file: %s", ec2.message);
  //   }
  //   return std::make_optional(std::move(pr));
  // }
  return std::nullopt;
}
} // namespace hazel