///
#ifndef WINUTILS_WINUI_HPP
#define WINUTILS_WINUI_HPP

#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif

namespace winutils {

struct Button {
  HWND hWnd;
  RECT layout;
  void Visible(BOOL v) { EnableWindow(hWnd, v); }
  bool IsVisible() const { return IsWindowVisible(hWnd) == TRUE; }
  bool UpdateWindowPos(int dpiX, HFONT hFont) {
    ::SetWindowPos(hWnd, NULL, MulDiv(r.left, dpiX, 96),
                   MulDiv(r.top, dpiX, 96), MulDiv(r.right - r.left, dpiX, 96),
                   MulDiv(r.bottom - r.top, dpiX, 96),
                   SWP_NOZORDER | SWP_NOACTIVATE);
    ::SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, lParam);
    return true;
  }
};

struct TextBox {
  HWND hWnd;
  RECT layout;
  std::wstring Content() {
    auto l = GetWindowTextLengthW(hInput);
    if (l == 0 || l > 0x8000) {
      return L"";
    }
    std::wstring s(l + 1, L'\0');
    auto k = GetWindowTextW(hWnd, &s[0], l + 1); //// Null T
    s.resize(k);
    return s;
  }
  bool UpdateWindowPos(int dpiX, HFONT hFont) {
    ::SetWindowPos(hWnd, NULL, MulDiv(r.left, dpiX, 96),
                   MulDiv(r.top, dpiX, 96), MulDiv(r.right - r.left, dpiX, 96),
                   MulDiv(r.bottom - r.top, dpiX, 96),
                   SWP_NOZORDER | SWP_NOACTIVATE);
    ::SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, lParam);
    return true;
  }
};
} // namespace winutils

#endif
