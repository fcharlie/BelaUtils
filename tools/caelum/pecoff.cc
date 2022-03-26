///
#include "caelum.hpp"
#include "window.hpp"
#include <bela/picker.hpp>
#include <CommCtrl.h>
#include "pe.hpp"

namespace caelum {
// Fill

inline std::wstring flatvector(const std::vector<std::wstring> &v, std::wstring_view delimiter = L", ") {
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

inline void BindFileVersion(const bela::pe::Version &ver, caelum::AttributesTables &at) {
  if (!ver.FileDescription.empty() && ver.FileDescription.size() < 50) {
    at.Append(L"FileDescription:", ver.FileDescription);
    return;
  }
  if (!ver.ProductName.empty() && ver.ProductName.size() < 50) {
    at.Append(L"ProductName:", ver.ProductName);
    return;
  }
}

bool Window::InquisitivePE() {
  auto path = wUrl.Content();
  bela::error_code ec;
  bela::pe::File file;

  if (!file.NewFile(path, ec)) {
    bela::BelaMessageBox(hWnd, L"Inquisitive PE ", ec.message.data(), nullptr, bela::mbs_t::FATAL);
    return false;
  }
  bela::pe::FunctionTable ft;
  if (!file.LookupFunctionTable(ft, ec)) {
    bela::BelaMessageBox(hWnd, L"Inquisitive PE ", ec.message.data(), nullptr, bela::mbs_t::FATAL);
    return false;
  }
  if (auto vi = bela::pe::Lookup(path, ec); vi) {
    BindFileVersion(*vi, tables);
  }
  tables.Append(L"Machine:", caelum::Machine(static_cast<uint32_t>(file.Machine())));
  tables.Append(L"Subsystem:", caelum::Subsystem(static_cast<uint32_t>(file.Subsystem())));

  uint16_t dllcharacteristics{0};
  auto &oh = file.Header();
  dllcharacteristics = oh.DllCharacteristics;
  tables.Append(L"OS Version:", bela::StringCat(oh.MajorOperatingSystemVersion, L".", oh.MinorOperatingSystemVersion));
  tables.Append(L"Link Version:", bela::StringCat(oh.MajorLinkerVersion, L".", oh.MajorLinkerVersion));
  if (auto meta = file.LookupDotNetMetadata(ec); meta) {
    tables.Append(L"CLR Details:", bela::encode_into<char, wchar_t>(meta->version));
  }
  tables.Append(L"Characteristics:");
  tables.Append(L"Depends:");

  auto y = 80 + 30 * tables.ats.size();
  auto charsv = caelum::Characteristics(file.Fh().Characteristics, dllcharacteristics);
  // depends lab append
  std::wstring depends;
  for (auto &im : ft.imports) {
    bela::StrAppend(&depends, bela::encode_into<char, wchar_t>(im.first), L" (", im.second.size(), L")\r\n");
  }
  for (auto &im : ft.delayimprots) {
    bela::StrAppend(&depends, L"(Delay) ", bela::encode_into<char, wchar_t>(im.first), L" (", im.second.size(),
                    L")\r\n");
  }
  auto strcharsv = flatvector(charsv);
  constexpr auto es =
      WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY;
  constexpr auto exs = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY;
  CreateSubWindow(exs, WC_EDITW, strcharsv.data(), es, 185, (int)y, 460, 60, nullptr, wCharacteristics);
  CreateSubWindow(exs, WC_EDITW, depends.data(), es, 185, (int)y + 65, 460, 80, nullptr, wDepends);
  return true;
}

} // namespace caelum