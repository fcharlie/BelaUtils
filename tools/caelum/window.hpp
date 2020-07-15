#ifndef CAELUM_WINDOW_HPP
#define CALEUM_WINDOW_HPP
#include "caelum.hpp"
#include <windowsx.h>
#include <d2d1helper.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <string>
#include <string_view>
#include <vector>
#include <cassert>
#include <optional>
#include <mutex>
#include <bela/base.hpp>

namespace caelum {
namespace ui {
enum WidgetNumber : ptrdiff_t {
  about = 1001, //
  editor = 1002,
  picker = 1003,
  characteristics = 1004,
  depends = 1005
};
};

struct AttributesTable {
  std::wstring name;
  std::wstring value;
};

struct Label {
  Label() = default;
  Label(std::wstring_view sv, LONG left, LONG top, LONG right, LONG bottom) : content(sv) {
    mlayout = D2D1::RectF((float)left, (float)top, (float)right, (float)bottom);
  }
  const wchar_t *data() const { return content.data(); }
  uint32_t length() const { return static_cast<uint32_t>(content.size()); }
  D2D_RECT_F layout() const { return mlayout; }
  std::wstring content;
  D2D1_RECT_F mlayout;
};

struct AttributesTables {
  std::vector<AttributesTable> ats;
  std::vector<std::wstring> names;
  std::size_t mnlen{0};
  bool Empty() const { return ats.empty() && names.empty(); }
  bool HasDepends() const { return !names.empty(); }
  std::wstring_view Characteristics() const { return names[0]; }
  std::wstring_view Depends() const { return names[1]; }
  AttributesTables &Clear() {
    mnlen = 0;
    ats.clear();
    names.clear();
    return *this;
  }
  AttributesTables &Append(std::wstring_view name, std::wstring &&value) {
    mnlen = (std::max)(mnlen, name.size());
    ats.emplace_back(AttributesTable{std::wstring(name), std::move(value)});
    return *this;
  }
  AttributesTables &Append(std::wstring_view name, std::wstring_view value) {
    mnlen = (std::max)(mnlen, name.size());
    ats.emplace_back(AttributesTable{std::wstring(name), std::wstring(value)});
    return *this;
  }
  AttributesTables &Append(std::wstring_view name) {
    mnlen = (std::max)(mnlen, name.size());
    names.emplace_back(name);
    return *this;
  }
};

struct Widget {
  HWND hWnd{nullptr};
  int X{0};
  int Y{0};
  int W{0};
  int H{0};
  bool Alived() const { return hWnd != nullptr; }
  void Destroy() {
    if (hWnd != nullptr) {
      ::DestroyWindow(hWnd);
      hWnd = nullptr;
    }
  }
  void Visible(BOOL v) { EnableWindow(hWnd, v); }
  bool IsVisible() const { return IsWindowVisible(hWnd) == TRUE; }
  std::wstring Content() {
    auto n = GetWindowTextLengthW(hWnd);
    if (n <= 0) {
      return L"";
    }
    std::wstring str;
    size_t l = n + 1;
    str.resize(l);
    auto k = GetWindowTextW(hWnd, str.data(), static_cast<int>(l));
    str.resize(k);
    return str;
  }
  void Content(std::wstring_view text) {
    //
    ::SetWindowTextW(hWnd, text.data());
  }
};

class Window {
private:
  static Window *GetThisFromHandle(HWND const window) noexcept {
    return reinterpret_cast<Window *>(GetWindowLongPtr(window, GWLP_USERDATA));
  }
  LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnCreate(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnSize(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnPaint(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnDpiChanged(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnDropfiles(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnStaticColor(WPARAM const wparam, LPARAM const lparam) noexcept;
  ///
  LRESULT DoAbout(WORD wNotifyCode);
  LRESULT DoPicker(WORD wNotifyCode);
  // LRESULT is LONG_PTR so when AMD64 8byte long (can as pointer)
  // HRESULT is LONG when AMD64 4byte
  HRESULT CreateDeviceResources();
  void DiscardDeviceResources();
  HRESULT OnRender();
  void AttributesTablesDraw();
  bool InquisitivePE();
  bool ResolveLink(std::wstring_view file);
  bool CreateSubWindow(DWORD dwStyleEx, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle,
                       int X, int Y, int nWidth, int nHeight, HMENU hMenu, Widget &w) {
    auto hw = CreateWindowExW(dwStyleEx, lpClassName, lpWindowName, dwStyle, MulDiv(X, dpiX, 96),
                              MulDiv(Y, dpiX, 96), MulDiv(nWidth, dpiX, 96),
                              MulDiv(nHeight, dpiX, 96), hWnd, hMenu, hInst, nullptr);
    if (hw == nullptr) {
      return false;
    }
    w.hWnd = hw;
    w.X = X;
    w.Y = Y;
    w.W = nWidth;
    w.H = nHeight;
    ::SendMessageW(hw, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }

  bool UpdateWidgetPos(const Widget &w) {
    if (w.hWnd == nullptr) {
      return false;
    }
    ::SetWindowPos(w.hWnd, NULL, MulDiv(w.X, dpiX, 96), MulDiv(w.Y, dpiX, 96),
                   MulDiv(w.W, dpiX, 96), MulDiv(w.H, dpiX, 96), SWP_NOZORDER | SWP_NOACTIVATE);
    ::SendMessageW(w.hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }

public:
  Window();
  ~Window();
  bool MakeWindow();
  void RunMessageLoop();
  static LRESULT WINAPI WindowProc(HWND const window, UINT const message, WPARAM const wparam,
                                   LPARAM const lparam) noexcept;

private:
  HWND hWnd{nullptr};
  HINSTANCE hInst{nullptr};
  // ---
  ID2D1Factory7 *factory{nullptr};
  IDWriteFactory7 *dwFactory{nullptr};
  IDWriteTextFormat3 *dwFormat{nullptr};
  //
  ID2D1HwndRenderTarget *renderTarget{nullptr};
  ID2D1SolidColorBrush *textBrush{nullptr};
  ID2D1SolidColorBrush *streaksbrush{nullptr};
  //
  HBRUSH hbrBkgnd{nullptr};
  HFONT hFont{nullptr};
  //
  Widget wUrl;
  Widget wPicker;
  Widget wCharacteristics;
  Widget wDepends;
  Widget wDelayDepends;
  //
  std::vector<Label> labels;
  AttributesTables tables;
  std::vector<Label> depends;
  //
  std::mutex mtx;
  //
  int dpiX{96};
  int dpiY{96};
};
} // namespace caelum

#endif