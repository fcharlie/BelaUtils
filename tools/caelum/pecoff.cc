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
  for (auto &i : v) {
    total += i.size() + delimiter.size();
  }
  s.resize(total);
  for (auto &i : v) {
    s.append(i).append(delimiter);
  }
  if (!s.empty()) {
    s.resize(s.size() - delimiter.size());
  }
  return s;
}

bool Window::InquisitivePE() {
  auto path = wUrl.Content();
  bela::error_code ec;
  auto pea = bela::PESimpleDetailsAze(path, ec);
  if (!pea) {
    bela::BelaMessageBox(hWnd, L"Inquisitive PE ", ec.message.data(), nullptr,
                         bela::mbs_t::FATAL);
    return false;
  }
  tables.Append(L"Machine:",
                caelum::Machine(static_cast<uint32_t>(pea->machine)));
  tables.Append(L"Subsystem:",
                caelum::Subsystem(static_cast<uint32_t>(pea->subsystem)));
  tables.Append(L"OS Version:", pea->osver.ToString());
  tables.Append(L"Link Version:", pea->linkver.ToString());
  if (!pea->clrmsg.empty()) {
    tables.Append(L"CLR Details:", pea->clrmsg);
  }

  tables.Append(L"Characteristics");
  tables.Append(L"Depends");

  auto y = 80 + 30 * tables.ats.size() * dpiX / 96;
  auto charsv =
      caelum::Characteristics(pea->characteristics, pea->dllcharacteristics);
  // depends lab append

  auto strcharsv = flatvector(charsv);
  auto depends = flatvector(pea->depends);
  if (!pea->delays.empty()) {
    auto delaydepends = flatvector(pea->delays);
    bela::StrAppend(&depends, L"\r\nDelay:\r\n", delaydepends, L"\r\n");
  } else {
    bela::StrAppend(&depends, L"\r\n");
  }

  constexpr auto es = WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
                      ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY;
  constexpr auto exs = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR |
                       WS_EX_NOPARENTNOTIFY;
  CreateSubWindow(exs, WC_EDITW, strcharsv.data(), es, 185, (int)y, 460, 55,
                  nullptr, wCharacteristics);
  CreateSubWindow(exs, WC_EDITW, depends.data(), es, 185, (int)y + 60, 460, 80,
                  nullptr, wDepends);
  return true;
}

} // namespace caelum