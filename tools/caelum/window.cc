//
#include "window.hpp"
#include "resource.h"
//
#include <bela/picker.hpp>
#include <bela/path.hpp>
#include <shellapi.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace caelum {

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
  Free(&streaksbrush);
  if (hbrBkgnd != nullptr) {
    DeleteBrush(hbrBkgnd);
  }
  if (hFont != nullptr) {
    DeleteFont(hFont);
  }
}

template <class Factory>
HRESULT DWriteCreateFactory(_In_ DWRITE_FACTORY_TYPE factoryType,
                            _Out_ Factory **factory) {
  return ::DWriteCreateFactory(factoryType, __uuidof(Factory),
                               reinterpret_cast<IUnknown **>(factory));
}

bool Window::MakeWindow() {
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
  constexpr const auto wndclassname = L"Caelum.Window";
  WNDCLASSEXW wc{};
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hInstance = reinterpret_cast<HINSTANCE>(&__ImageBase);
  wc.lpszClassName = wndclassname;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = Window::WindowProc;
  // create Direct2D DWrite resources
  RegisterClassExW(&wc);
  CreateWindowExW(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, wc.lpszClassName,
                  wndclassname,
                  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN |
                      WS_MINIMIZEBOX,
                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                  nullptr, nullptr, wc.hInstance, this);

  return true;
}

HRESULT Window::CreateDeviceResources() {
  if (renderTarget != nullptr) {
    return S_OK;
  }
  HRESULT hr = S_OK;
  RECT rect;
  ::GetClientRect(hWnd, &rect);
  auto size = D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top);
  hr = factory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(hWnd, size), &renderTarget);
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  renderTarget->SetDpi(static_cast<float>(dpiX), static_cast<float>(dpiX));
  hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),
                                           &textBrush);
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(0xFFC300),
                                           &streaksbrush);
  return hr;
}

HRESULT Window::OnRender() {
  auto hr = CreateDeviceResources();
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  auto dsz = renderTarget->GetSize();
  renderTarget->BeginDraw();
  renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
  renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
  return S_OK;
}

void Window::AttributesTablesDraw() {
  if (tables.Empty()) {
    return; ///
  }
  ////////// -->
  float offset = 80;
  float w1 = 30;
  float w2 = 60;
  float keyoff = 180;
  float xoff = 60;
  for (const auto &e : tables.ats) {
    renderTarget->DrawTextW(e.name.c_str(), (UINT32)e.name.size(), dwFormat,
                            D2D1::RectF(xoff, offset, keyoff, offset + w1),
                            textBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
    renderTarget->DrawTextW(
        e.value.c_str(), (UINT32)e.value.size(), dwFormat,
        D2D1::RectF(keyoff + 10, offset, keyoff + 400, offset + w1), textBrush,
        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
        DWRITE_MEASURING_MODE_NATURAL);
    offset += w1;
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
    break;
  default:
    break;
  }
  return DefWindowProcW(hWnd, message, wparam, lparam);
}

LRESULT Window::OnCreate(WPARAM const wparam, LPARAM const lparam) noexcept {
  //
  return S_OK;
}

LRESULT Window::OnSize(WPARAM const wparam, LPARAM const lparam) noexcept {
  UINT width = LOWORD(lparam);
  UINT height = HIWORD(lparam);
  if (renderTarget != nullptr) {
    renderTarget->Resize(D2D1::SizeU(width, height));
  }
  return S_OK;
}

LRESULT Window::OnPaint(WPARAM const wparam, LPARAM const lparam) noexcept {
  PAINTSTRUCT ps;
  BeginPaint(hWnd, &ps);
  if (!SUCCEEDED(OnRender())) {
    //
  }
  EndPaint(hWnd, &ps);
  return S_OK;
}

LRESULT Window::OnDpiChanged(WPARAM const wparam,
                             LPARAM const lparam) noexcept {
  dpiX = static_cast<UINT32>(LOWORD(wparam));
  dpiX = static_cast<UINT32>(HIWORD(wparam));
  auto prcNewWindow = reinterpret_cast<RECT *const>(lparam);
  // resize window with new DPI
  ::SetWindowPos(hWnd, nullptr, prcNewWindow->left, prcNewWindow->top,
                 prcNewWindow->right - prcNewWindow->left,
                 prcNewWindow->bottom - prcNewWindow->top,
                 SWP_NOZORDER | SWP_NOACTIVATE);
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
  // TODO resolve link
  return S_OK;
}

// https://docs.microsoft.com/en-us/windows/win32/controls/wm-ctlcolorstatic
LRESULT Window::OnStaticColor(WPARAM const wparam,
                              LPARAM const lparam) noexcept {
  if (hbrBkgnd == nullptr) {
    hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
  }
  return reinterpret_cast<INT_PTR>(hbrBkgnd);
}

LRESULT Window::DoAbout(WORD wNotifyCode) {
  // show about
  bela::BelaMessageBox(hWnd, L"About Caelum \u2764 PE analysis tool",
                       FULL_COPYRIGHT, CAELUM_APPLINK, bela::mbs_t::ABOUT);
  return S_OK;
}

LRESULT Window::DoPicker(WORD wNotifyCode) {
  constexpr const bela::filter_t filters[] = {
      {L"Windows  Execute File (*.exe;*.com;*.dll;*.sys)",
       L"*.exe;*.com;*.dll;*.sys"},
      {L"Windows Other File (*.scr;*.fon;*.drv)", L"*.scr;*.fon;*.drv"},
      {L"All Files (*.*)", L"*.*"}};
  auto file =
      bela::FilePicker(hWnd, L"Select PE file", filters, std::size(filters));
  if (!file) {
    //
    return S_OK;
  }
  return S_OK;
}

} // namespace caelum