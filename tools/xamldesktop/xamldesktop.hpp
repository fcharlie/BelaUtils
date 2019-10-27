//
#ifndef XAMLDESKTOP_HPP
#define XANLDESKTOP_HPP
#pragma once

#include <ShellScalingAPI.h>

#include <winrt/Windows.System.h>
#include <winrt/windows.ui.xaml.hosting.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <winrt/windows.ui.xaml.controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

namespace xamldesktop {
template <typename T> struct DesktopWindow {
  static T *GetThisFromHandle(HWND const window) noexcept {
    return reinterpret_cast<T *>(GetWindowLongPtr(window, GWLP_USERDATA));
  }

  static LRESULT __stdcall WndProc(HWND const window, UINT const message,
                                   WPARAM const wparam,
                                   LPARAM const lparam) noexcept {
    WINRT_ASSERT(window);

    if (WM_NCCREATE == message) {
      auto cs = reinterpret_cast<CREATESTRUCT *>(lparam);
      T *that = static_cast<T *>(cs->lpCreateParams);
      WINRT_ASSERT(that);
      WINRT_ASSERT(!that->m_window);
      that->m_window = window;
      SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));

      EnableNonClientDpiScaling(window);
      m_currentDpi = GetDpiForWindow(window);
    } else if (T *that = GetThisFromHandle(window)) {
      return that->MessageHandler(message, wparam, lparam);
    }

    return DefWindowProc(window, message, wparam, lparam);
  }

  LRESULT MessageHandler(UINT const message, WPARAM const wparam,
                         LPARAM const lparam) noexcept {
    switch (message) {
    case WM_DPICHANGED: {
      return HandleDpiChange(m_window, wparam, lparam);
    }

    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }

    case WM_SIZE: {
      UINT width = LOWORD(lparam);
      UINT height = HIWORD(lparam);

      if (T *that = GetThisFromHandle(m_window)) {
        that->DoResize(width, height);
      }
    }
    }

    return DefWindowProc(m_window, message, wparam, lparam);
  }

  // DPI Change handler. on WM_DPICHANGE resize the window
  LRESULT HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam) {
    HWND hWndStatic = GetWindow(hWnd, GW_CHILD);
    if (hWndStatic != nullptr) {
      UINT uDpi = HIWORD(wParam);

      // Resize the window
      auto lprcNewScale = reinterpret_cast<RECT *>(lParam);

      SetWindowPos(hWnd, nullptr, lprcNewScale->left, lprcNewScale->top,
                   lprcNewScale->right - lprcNewScale->left,
                   lprcNewScale->bottom - lprcNewScale->top,
                   SWP_NOZORDER | SWP_NOACTIVATE);

      if (T *that = GetThisFromHandle(hWnd)) {
        that->NewScale(uDpi);
      }
    }
    return 0;
  }

  void NewScale(UINT dpi) {}

  void DoResize(UINT width, UINT height) {}

protected:
  using base_type = DesktopWindow<T>;
  HWND m_window = nullptr;
  inline static UINT m_currentDpi = 0;
};
} // namespace xamldesktop

#endif