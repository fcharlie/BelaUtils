////
#ifndef KISASUM_UI_HPP
#define KISASUM_UI_HPP
#include <bela/base.hpp>
#include <bela/endian.hpp>
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
#include <atomic>
#include "kisasumutils.hpp"

namespace kisasum {

namespace ui {
enum WidgetNumber : ptrdiff_t {
  theme = 1000,
  about = 1001, //
  uppercase = 1002,
  algo = 1003,
  clear = 1004,
  picker = 1005
};
};

// swap to LE
inline COLORREF calcLuminance(UINT32 cr) {
  cr = bela::swaple(cr);
  int r = (cr & 0xff0000) >> 16;
  int g = (cr & 0xff00) >> 8;
  int b = (cr & 0xff);
  return RGB(r, g, b);
}

struct Label {
  Label() = default;
  Label(std::wstring_view sv, LONG left, LONG top, LONG right, LONG bottom)
      : content(sv) {
    mlayout = D2D1::RectF((float)left, (float)top, (float)right, (float)bottom);
  }
  const wchar_t *data() const { return content.data(); }
  uint32_t length() const { return static_cast<uint32_t>(content.size()); }
  D2D_RECT_F layout() const { return mlayout; }
  std::wstring content;
  D2D1_RECT_F mlayout;
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
  bool IsChecked() const {
    return (hWnd != nullptr && Button_GetCheck(hWnd) == BST_CHECKED);
  }
  void Content(std::wstring_view text) { ::SetWindowTextW(hWnd, text.data()); }
  std::wstring Content() {
    auto n = GetWindowTextLengthW(hWnd);
    if (n <= 0) {
      return L"";
    }
    std::wstring str;
    str.resize(n + 1);
    auto k = GetWindowTextW(hWnd, str.data(), n + 1);
    str.resize(k);
    return str;
  }
};

class Window {
private:
  static Window *GetThisFromHandle(HWND const window) noexcept {
    return reinterpret_cast<Window *>(GetWindowLongPtr(window, GWLP_USERDATA));
  }
  LRESULT MessageHandler(UINT const message, WPARAM const wparam,
                         LPARAM const lparam) noexcept;
  LRESULT OnCreate(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnSize(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnPaint(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnDpiChanged(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnDropfiles(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnStaticColor(WPARAM const wparam, LPARAM const lparam) noexcept;
  ///
  LRESULT DoTheme(WORD wNotifyCode);
  LRESULT DoAbout(WORD wNotifyCode);
  //
  LRESULT DoClear(WORD wNotiftCode);
  LRESULT DoPicker(WORD wNotifyCode);
  //
  HRESULT CreateDeviceResources();
  void DiscardDeviceResources();
  HRESULT OnRender();
  //
  bool UpdateTheme();
  bool RefreshFont();

  //
  bool CreateSubWindow(DWORD dwStyleEx, LPCWSTR lpClassName,
                       LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y,
                       int nWidth, int nHeight, HMENU hMenu, Widget &w) {
    auto hw = CreateWindowExW(
        dwStyleEx, lpClassName, lpWindowName, dwStyle, MulDiv(X, dpiX, 96),
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

  bool UpdateWidgetPos(Widget &w) {
    if (w.hWnd == nullptr) {
      return false;
    }
    ::SetWindowPos(w.hWnd, NULL, MulDiv(w.X, dpiX, 96), MulDiv(w.Y, dpiX, 96),
                   MulDiv(w.W, dpiX, 96), MulDiv(w.H, dpiX, 96),
                   SWP_NOZORDER | SWP_NOACTIVATE);
    ::SendMessageW(w.hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }

public:
  Window();
  ~Window();
  bool MakeWindow();
  void RunMessageLoop();
  /// static
  static LRESULT WINAPI WindowProc(HWND const window, UINT const message,
                                   WPARAM const wparam,
                                   LPARAM const lparam) noexcept;

private:
  HWND hWnd{nullptr};
  HINSTANCE hInst{nullptr};
  // ---
  ID2D1Factory7 *factory{nullptr};
  IDWriteFactory7 *dwFactory{nullptr};
  IDWriteTextFormat3 *dwFormat{nullptr};
  IDWriteTextFormat3 *dwIconFormat{nullptr};
  //
  ID2D1HwndRenderTarget *renderTarget{nullptr};
  ID2D1SolidColorBrush *textBrush{nullptr};
  ID2D1SolidColorBrush *AppPageBackgroundThemeBrush{nullptr};
  //
  HBRUSH hbrBkgnd{nullptr};
  HFONT hFont{nullptr};
  //
  Widget wUppercase;
  Widget wAlgorithm;
  Widget wClear;
  Widget wPicker;
  KisasumOptions options;
  //
  int dpiX{96};
  int dpiY{96};
  //
  std::atomic_uint32_t progress{0};
};
} // namespace kisasum

#endif