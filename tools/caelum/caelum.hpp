//
#ifndef CAELUM_HPP
#define CAELUM_HPP

#include <SDKDDKVer.h>
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif
#include <string>
#include <optional>
#include <bela/base.hpp>

namespace caelum {
std::optional<std::wstring> ResolveLink(std::wstring_view file, bela::error_code &ec);
}

#endif