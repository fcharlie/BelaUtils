///
#ifndef WINUTILS_WINUI_HPP
#define WINUTILS_WINUI_HPP

#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif
#include <windowsx.h>
#include <CommCtrl.h>
#include <string>

namespace winutils {

struct Button {
  HWND hWnd;
  int X;
  int Y;
  int W;
  int H;
  bool Initialize(const wchar_t *name, int x, int y, int cx, int cy,
                  HINSTANCE hInst, int dpiX, int id, HWND hParent,
                  HFONT hFont) {
    X = x;
    Y = y;
    W = cx;
    H = cy;
    constexpr const auto bsex = WS_EX_LEFT | WS_EX_LTRREADING |
                                WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY;
    constexpr const auto bs =
        BS_PUSHBUTTON | BS_TEXT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE;
    hWnd = ::CreateWindowExW(bsex, WC_BUTTONW, name, bs, MulDiv(X, dpiX, 96),
                             MulDiv(Y, dpiX, 96), MulDiv(W, dpiX, 96),
                             MulDiv(H, dpiX, 96), hParent, (HMENU)id, hInst,
                             nullptr);
    if (hWnd == nullptr) {
      return false;
    }
    ::SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }
  void Visible(BOOL v) { EnableWindow(hWnd, v); }
  bool IsVisible() const { return IsWindowVisible(hWnd) == TRUE; }
  bool UpdateWindowPos(int dpiX, HFONT hFont) {
    ::SetWindowPos(hWnd, nullptr, MulDiv(X, dpiX, 96), MulDiv(Y, dpiX, 96),
                   MulDiv(W, dpiX, 96), MulDiv(H, dpiX, 96),
                   SWP_NOZORDER | SWP_NOACTIVATE);
    ::SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }
};

struct TextBox {
  HWND hWnd;
  int X;
  int Y;
  int W;
  int H;
  bool Initialize(const wchar_t *text, int x, int y, int cx, int cy,
                  HINSTANCE hInst, int dpiX, int id, HWND hParent,
                  HFONT hFont) {
    constexpr const auto eex = WS_EX_LEFT | WS_EX_LTRREADING |
                               WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY |
                               WS_EX_CLIENTEDGE;
    constexpr const auto es = WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE |
                              WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
    X = x;
    Y = y;
    W = cx;
    H = cy;
    hWnd = ::CreateWindowExW(eex, WC_EDITW, text, es, MulDiv(X, dpiX, 96),
                             MulDiv(Y, dpiX, 96), MulDiv(W, dpiX, 96),
                             MulDiv(H, dpiX, 96), hParent, (HMENU)id, hInst,
                             nullptr);
    if (hWnd == nullptr) {
      return false;
    }
    ::SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }
  void Visible(BOOL v) { EnableWindow(hWnd, v); }
  bool IsVisible() const { return IsWindowVisible(hWnd) == TRUE; }
  std::wstring Content() {
    auto l = GetWindowTextLengthW(hWnd);
    if (l == 0 || l > 0x8000) {
      return L"";
    }
    std::wstring s(l + 1, L'\0');
    auto k = GetWindowTextW(hWnd, &s[0], l + 1); //// Null T
    s.resize(k);
    return s;
  }
  bool UpdateWindowPos(int dpiX, HFONT hFont) {
    ::SetWindowPos(hWnd, nullptr, MulDiv(X, dpiX, 96), MulDiv(Y, dpiX, 96),
                   MulDiv(W, dpiX, 96), MulDiv(H, dpiX, 96),
                   SWP_NOZORDER | SWP_NOACTIVATE);
    ::SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }
};

struct Progress {
  HWND hWnd;
  int X;
  int Y;
  int W;
  int H;
  bool Initialize(int x, int y, int cx, int cy, HINSTANCE hInst, int dpiX,
                  int id, HWND hParent, HFONT hFont) {
    constexpr const auto psex = WS_EX_LEFT | WS_EX_LTRREADING |
                                WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY;
    constexpr const auto ps = WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE;
    X = x;
    Y = y;
    W = cx;
    H = cy;
    hWnd = ::CreateWindowExW(psex, PROGRESS_CLASSW, L"", ps,
                             MulDiv(X, dpiX, 96), MulDiv(Y, dpiX, 96),
                             MulDiv(W, dpiX, 96), MulDiv(H, dpiX, 96), hParent,
                             (HMENU)id, hInst, nullptr);
    if (hWnd == nullptr) {
      return false;
    }
    ::SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }
  void Visible(BOOL v) { EnableWindow(hWnd, v); }
  bool IsVisible() const { return IsWindowVisible(hWnd) == TRUE; }
  bool UpdateWindowPos(int dpiX, HFONT hFont) {
    ::SetWindowPos(hWnd, nullptr, MulDiv(X, dpiX, 96), MulDiv(Y, dpiX, 96),
                   MulDiv(W, dpiX, 96), MulDiv(H, dpiX, 96),
                   SWP_NOZORDER | SWP_NOACTIVATE);
    ::SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }
  void Refresh() {
    //
  }
  bool UpdateRate(int rate, int total) {
    ::SendMessageW(hWnd, PBM_SETRANGE32, 0, total);
    ::SendMessageW(hWnd, PBM_SETPOS, 0, rate);
    return true;
  }
};

} // namespace winutils

#endif
