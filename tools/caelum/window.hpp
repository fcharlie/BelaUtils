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

namespace caelum {
namespace ui {
enum WidgetNumber : ptrdiff_t {
  about = 1001, //
  picker = 1002,
  characteristics = 1003,
  depends = 1004
};
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
  LRESULT OnDisplayChange(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnDropfiles(WPARAM const wparam, LPARAM const lparam) noexcept;
  LRESULT OnStaticColor(WPARAM const wparam, LPARAM const lparam) noexcept;
  ///
  LRESULT DoAbout(WORD wNotifyCode);
  LRESULT DoPicker(WORD wNotifyCode);

public:
  Window();
  ~Window();
  bool MakeWindow();
  static LRESULT WINAPI WindowProc(HWND const window, UINT const message,
                                   WPARAM const wparam,
                                   LPARAM const lparam) noexcept;

private:
  HWND hWnd{nullptr};
  HBRUSH hbrBkgnd{nullptr};
  // ---
  ID2D1Factory7 *factory{nullptr};
  IDWriteFactory7 *dwFactory{nullptr};
  IDWriteTextFormat3 *dwFormat{nullptr};
  //
  ID2D1HwndRenderTarget *renderTarget{nullptr};
  ID2D1SolidColorBrush *textBrush{nullptr};
  ID2D1SolidColorBrush *streaksbrush{nullptr};

  //
  int dpiX{96};
  int dpiY{96};
};
} // namespace caelum

#endif