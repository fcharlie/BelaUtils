///
#include "caelum.hpp"
#include "window.hpp"
#include <bela/picker.hpp>
#include <bela/strcat.hpp>
#include <CommCtrl.h>
#include "pe.hpp"

namespace caelum {
// Fill

inline std::wstring flatvector(const std::vector<std::wstring> &v,
                               std::wstring_view delimiter = L", ") {
  std::wstring s;
  size_t total = 0;
  for (const auto &i : v) {
    total += i.size() + delimiter.size();
  }
  s.reserve(total);
  for (const auto &i : v) {
    s.append(i).append(delimiter);
  }
  constexpr std::wstring_view newline = L"\r\n";
  if (!s.empty() && delimiter != newline) {
    s.resize(s.size() - delimiter.size());
  }
  return s;
}

bool Window::InquisitivePE() {
  auto path = wUrl.Content();
  bela::error_code ec;
  auto pea = bela::pe::Expose(path, ec);
  if (!pea) {
    bela::BelaMessageBox(hWnd, L"Inquisitive PE ", ec.message.data(), nullptr,
                         bela::mbs_t::FATAL);
    return false;
  }
  auto vi = bela::pe::ExposeVersion(path, ec);
  bool hasProductName = false;
  if (vi) {
    tables.Append(L"ProductName:", vi->ProductName);
  }
  tables.Append(L"Machine:",
                caelum::Machine(static_cast<uint32_t>(pea->machine)));
  tables.Append(L"Subsystem:",
                caelum::Subsystem(static_cast<uint32_t>(pea->subsystem)));
  tables.Append(L"OS Version:", pea->osver.Str());
  tables.Append(L"Link Version:", pea->linkver.Str());
  if (!pea->clrmsg.empty()) {
    tables.Append(L"CLR Details:", pea->clrmsg);
  }

  tables.Append(L"Characteristics:");
  tables.Append(L"Depends:");

  auto y = 80 + 30 * tables.ats.size();
  if (hasProductName) {
    y += 30;
  }
  auto charsv =
      caelum::Characteristics(pea->characteristics, pea->dllcharacteristics);
  // depends lab append

  auto strcharsv = flatvector(charsv);
  auto depends = flatvector(pea->depends, L"\r\n");
  if (!pea->delays.empty()) {
    auto delaydepends = flatvector(pea->delays, L"\r\n");
    bela::StrAppend(&depends, L"**below is delay**\r\n", delaydepends);
  }

  constexpr auto es = WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
                      ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY;
  constexpr auto exs = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR |
                       WS_EX_NOPARENTNOTIFY;
  CreateSubWindow(exs, WC_EDITW, strcharsv.data(), es, 185, (int)y, 460, 60,
                  nullptr, wCharacteristics);
  CreateSubWindow(exs, WC_EDITW, depends.data(), es, 185, (int)y + 65, 460, 80,
                  nullptr, wDepends);
  return true;
}

} // namespace caelum