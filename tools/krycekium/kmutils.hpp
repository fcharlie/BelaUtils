//
#ifndef KRYCEKIUM_UTILS_HPP
#define KRYCEKIUM_UTILS_HPP
#include <optional>
#include <bela/path.hpp>

namespace krycekium {
bool FolderIsEmpty(std::wstring_view dir);
bool IsSuffixEnabled(std::wstring_view msi);
std::optional<std::wstring> SameFolder(std::wstring_view msi);
} // namespace krycekium

#endif