///
#include "hazelinc.hpp"

namespace hazel {
bool IsDebugMode = false;
typedef hazel::internal::status_t (*explore_handle_t)(bela::MemView mv, particulars_result &pr);

std::optional<particulars_result> explore_file(std::wstring_view file, bela::error_code &ec) {
  using namespace hazel::internal;
  bela::MapView mmv;
  if (!mmv.MappingView(file, ec, 1, 32 * 1024)) {
    return std::nullopt;
  }
  auto mv = mmv.subview();
  particulars_result pr;
  constexpr const explore_handle_t handles[] = {
      // handles
      explore_binobj, explore_fonts,    explore_zip_family, explore_docs, explore_images, explore_archives,
      explore_media,  explore_git_file, explore_shlink,     explore_text, explore_chardet
      //
  };
  for (auto h : handles) {
    if (h(mv, pr) == internal::Found) {
      return std::make_optional<particulars_result>(std::move(pr));
    }
  }
  return std::nullopt;
}
} // namespace hazel