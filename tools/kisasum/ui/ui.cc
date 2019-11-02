////
#include "ui.hpp"
#include <commdlg.h>
#include <bela/picker.hpp>
#include <shellapi.h>
#include "resource.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace kisasum {

template <typename T> void Free(T *t) {
  if (*t != nullptr) {
    (*t)->Release();
    *t = nullptr;
  }
}

template <typename T> void FreeObj(T *t) {
  if (*t != nullptr) {
    DeleteObject(*t);
    *t = nullptr;
  }
}

static inline int Year() {
  SYSTEMTIME stime;
  GetSystemTime(&stime);
  return stime.wYear;
}

bool RefreshFont(HFONT &hFont, int dpiY) {
  if (hFont == nullptr) {
    hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  }
  LOGFONTW logFont = {0};
  if (GetObjectW(hFont, sizeof(logFont), &logFont) == 0) {
    return false;
  }
  logFont.lfHeight = -MulDiv(14, dpiY, 96);
  logFont.lfWeight = FW_NORMAL;
  wcscpy_s(logFont.lfFaceName, L"Segoe UI");
  auto hNewFont = CreateFontIndirectW(&logFont);
  if (hNewFont == nullptr) {
    return false;
  }
  DeleteObject(hFont);
  hFont = hNewFont;
  return true;
}

Window::Window() {
  //
  hInst = reinterpret_cast<HINSTANCE>(&__ImageBase);
}

Window::~Window() {
  Free(&factory);
  Free(&dwFactory);
  Free(&dwFormat);
  Free(&renderTarget);
  Free(&textBrush);
  Free(&AppPageBackgroundThemeBrush);
  if (hbrBkgnd != nullptr) {
    DeleteBrush(hbrBkgnd);
  }
  if (hFont != nullptr) {
    DeleteFont(hFont);
  }
}

template <class Factory>
[[nodiscard]] HRESULT DWriteCreateFactory(_In_ DWRITE_FACTORY_TYPE factoryType,
                                          _Out_ Factory **factory) {
  return ::DWriteCreateFactory(factoryType, __uuidof(Factory),
                               reinterpret_cast<IUnknown **>(factory));
}

bool Window::MakeWindow() {
  InitializeKisasumOptions(options);
  // initialize d2d1 factory and dwrite factory
  if (!SUCCEEDED(
          D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory))) {
    return false;
  }
  if (!SUCCEEDED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, &dwFactory))) {
    return false;
  }
  if (!SUCCEEDED(dwFactory->CreateTextFormat(
          L"Segeo UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
          DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"zh-CN",
          reinterpret_cast<IDWriteTextFormat **>(&dwFormat)))) {
    return false;
  }
  constexpr const auto wndclassname = L"Kisasum.Window";
  WNDCLASSEXW wc{};
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hInstance = hInst;
  wc.lpszClassName = wndclassname;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = Window::WindowProc;
  wc.cbSize = sizeof(wc);
  // create Direct2D DWrite resources
  auto atom = RegisterClassExW(&wc);
  if (atom == 0) {
    return false;
  }
  auto hWnd_ =
      CreateWindowExW(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, wc.lpszClassName,
                      L"Kisasum Immersive",
                      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                          WS_CLIPCHILDREN | WS_MINIMIZEBOX,
                      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                      CW_USEDEFAULT, nullptr, nullptr, wc.hInstance, this);

  return hWnd_ != nullptr;
}

void Window::RunMessageLoop() {
  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);
  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
}

LRESULT WINAPI Window::WindowProc(HWND const window, UINT const message,
                                  WPARAM const wparam,
                                  LPARAM const lparam) noexcept {
  assert(window);
  if (WM_NCCREATE == message) {
    auto cs = reinterpret_cast<CREATESTRUCT *>(lparam);
    Window *that = static_cast<Window *>(cs->lpCreateParams);
    assert(that);
    assert(!that->hWnd);
    that->hWnd = window;
    SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
  } else if (Window *that = GetThisFromHandle(window)) {
    return that->MessageHandler(message, wparam, lparam);
  }
  return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT Window::MessageHandler(UINT const message, WPARAM const wparam,
                               LPARAM const lparam) noexcept {
  switch (message) {
  case WM_CREATE:
    return OnCreate(wparam, lparam);
  case WM_DPICHANGED:
    return OnDpiChanged(wparam, lparam);
  case WM_PAINT:
    return OnPaint(wparam, lparam);
  case WM_SIZE:
    return OnSize(wparam, lparam);
  case WM_DROPFILES:
    return OnDropfiles(wparam, lparam);
  case WM_CTLCOLORSTATIC:
    return OnStaticColor(wparam, lparam);
  case WM_CLOSE:
    // send WM_DESTORY and WM_NCDESTROY
    DestroyWindow(hWnd);
    return S_OK;
  case WM_DESTROY:
    PostQuitMessage(0);
    return S_OK;
  case WM_SYSCOMMAND:
    if (ui::about == static_cast<ptrdiff_t>(LOWORD(wparam))) {
      return DoAbout(HIWORD(wparam));
    }
    break;
  case WM_COMMAND:
    if (ui::picker == static_cast<ptrdiff_t>(LOWORD(wparam))) {
      return DoPicker(HIWORD(wparam));
    }
    if (ui::clear == static_cast<ptrdiff_t>(LOWORD(wparam))) {
      return DoClear(HIWORD(wparam));
    }
    break;
  default:
    break;
  }
  return DefWindowProcW(hWnd, message, wparam, lparam);
}

LRESULT Window::OnCreate(WPARAM const wparam, LPARAM const lparam) noexcept {
  return S_OK;
}

LRESULT Window::OnSize(WPARAM const wparam, LPARAM const lparam) noexcept {
  return S_OK;
}

LRESULT Window::OnPaint(WPARAM const wparam, LPARAM const lparam) noexcept {
  return S_OK;
}

LRESULT Window::OnDpiChanged(WPARAM const wparam,
                             LPARAM const lparam) noexcept {
  return S_OK;
}

LRESULT Window::OnDropfiles(WPARAM const wparam, LPARAM const lparam) noexcept {
  auto hDrop = reinterpret_cast<HDROP>(wparam);
  auto n = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
  if (n == 0) {
    return S_OK;
  }
  std::wstring buf;
  buf.resize(0x8000);
  auto l = DragQueryFileW(hDrop, 0, buf.data(), 0x8000);
  DragFinish(hDrop);
  if (l == 0) {
    return S_OK;
  }
  buf.resize(l);
  return S_OK;
}

LRESULT Window::OnStaticColor(WPARAM const wparam,
                              LPARAM const lparam) noexcept {
  if (hbrBkgnd == nullptr) {
    hbrBkgnd = CreateSolidBrush(calcLuminance(options.panelcolor));
  }
  return reinterpret_cast<INT_PTR>(hbrBkgnd);
}

bool Window::UpdateTheme() {
  FreeObj(&hbrBkgnd);
  Free(&AppPageBackgroundThemeBrush);
  auto hr = renderTarget->CreateSolidColorBrush(
      D2D1::ColorF((UINT32)options.panelcolor), &AppPageBackgroundThemeBrush);
  InvalidateRect(wUppercase.hWnd, nullptr, TRUE);
  InvalidateRect(hWnd, nullptr, TRUE);
  // flush options
  return FlushKisasumOptions(options);
}

//
LRESULT Window::DoTheme(WORD wNotifyCode) {
  static constexpr COLORREF colors[] = {
      RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
      RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
      RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
      RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
      RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
      RGB(255, 255, 255),
  };

  CHOOSECOLORW co;
  ZeroMemory(&co, sizeof(co));
  co.lStructSize = sizeof(CHOOSECOLORW);
  co.hwndOwner = hWnd;
  co.lpCustColors = (LPDWORD)colors;
  co.rgbResult = calcLuminance(options.panelcolor);
  co.lCustData = 0;
  co.lpTemplateName = nullptr;
  co.lpfnHook = nullptr;
  co.Flags = CC_FULLOPEN | CC_RGBINIT;
  if (ChooseColorW(&co)) {
    auto r = GetRValue(co.rgbResult);
    auto g = GetGValue(co.rgbResult);
    auto b = GetBValue(co.rgbResult);
    options.panelcolor = (r << 16) + (g << 8) + b;
  }
  return S_OK;
}

///
LRESULT Window::DoAbout(WORD wNotifyCode) {
  bela::BelaMessageBox(hWnd, L"About Kisasum Hash Utilities", FULL_COPYRIGHT,
                       KISASUM_APPLINK, bela::mbs_t::ABOUT);
  return S_OK;
}

LRESULT Window::DoClear(WORD wNotiftCode) {
  //
  return S_OK;
}

LRESULT Window::DoPicker(WORD wNotifyCode) {
  //
  return S_OK;
}

//
HRESULT Window::CreateDeviceResources() {
  //
  return S_OK;
}

void Window::DiscardDeviceResources() {
  //
}

HRESULT Window::OnRender() {
  //
  return S_OK;
}

} // namespace kisasum