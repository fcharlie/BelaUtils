//
#include "window.hpp"
#include "resource.h"
//
#include <CommCtrl.h>
#include <bela/picker.hpp>
#include <bela/path.hpp>
#include <shellapi.h>
#include "caelum.hpp"

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
                      L"Caelum \u2764 PE Analyzer ",
                      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                          WS_CLIPCHILDREN | WS_MINIMIZEBOX,
                      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                      CW_USEDEFAULT, nullptr, nullptr, wc.hInstance, this);

  return hWnd_ != nullptr;
}

void Window::RunMessageLoop() {
  //
  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);
  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
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

void Window::DiscardDeviceResources() {
  //
  Free(&renderTarget);
  Free(&textBrush);
  Free(&streaksbrush);
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
  AttributesTablesDraw();
  for (const auto &l : labels) {
    renderTarget->DrawTextW(l.data(), l.length(), dwFormat, l.layout(),
                            textBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
  }
  hr = renderTarget->EndDraw();
  if (hr == D2DERR_RECREATE_TARGET) {
    hr = S_OK;
    DiscardDeviceResources();
  }
  return hr;
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
  if (!tables.HasDepends()) {
    return;
  }
  auto ch = tables.Characteristics();
  auto depends = tables.Depends();
  renderTarget->DrawTextW(ch.data(), (UINT32)ch.size(), dwFormat,
                          D2D1::RectF(xoff, offset, keyoff, offset + w2),
                          textBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                          DWRITE_MEASURING_MODE_NATURAL);
  renderTarget->DrawTextW(
      depends.data(), (UINT32)depends.size(), dwFormat,
      D2D1::RectF(xoff, offset + w2, keyoff, offset + w2 + w2), textBrush,
      D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT, DWRITE_MEASURING_MODE_NATURAL);
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
  dpiX = GetDpiForWindow(hWnd);
  dpiY = dpiX;
  RECT rect;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
  int cx = rect.right - rect.left;
  auto w = MulDiv(720, dpiX, 96);
  SetWindowPos(hWnd, nullptr, (cx - w) / 2, MulDiv(100, dpiX, 96), w,
               MulDiv(500, dpiX, 96), SWP_NOZORDER | SWP_NOACTIVATE);
  RefreshFont(hFont, dpiX);
  if (hFont == nullptr) {
    hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  }
  HICON hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_CAELUM_ICON));
  SendMessageW(hWnd, WM_SETICON, TRUE, (LPARAM)hIcon);
  ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
  ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
  ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
  DragAcceptFiles(hWnd, TRUE);

  // Initialize Labels
  labels.emplace_back(L"PE:", 20, 40, 100, 65);
  /// Copyrigth
  labels.emplace_back(bela::StringCat(L"\U0001f60b Copyright \u00a9 ", Year(),
                                      L". Force Charlie. All Rights Reserved."),
                      60, 400, 600, 420);
  HMENU hSystemMenu = ::GetSystemMenu(hWnd, FALSE);
  InsertMenuW(hSystemMenu, SC_CLOSE, MF_ENABLED, ui::about,
              L"About Caelum \u2764 PE analysis tool\tAlt+F1");

  constexpr const auto eex = WS_EX_LEFT | WS_EX_LTRREADING |
                             WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY |
                             WS_EX_CLIENTEDGE;
  constexpr const auto es = WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE |
                            WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
  constexpr const auto bex = WS_EX_LEFT | WS_EX_LTRREADING |
                             WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY;
  constexpr const auto bs =
      BS_PUSHBUTTON | BS_TEXT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE;

  CreateSubWindow(eex, WC_EDITW, L"", es, 60, 40, 510, 27, HMENU(ui::editor),
                  wUrl);
  CreateSubWindow(bex, WC_BUTTONW, L"...", bs, 575, 40, 65, 27,
                  HMENU(ui::picker), wPicker);

  int numArgc = 0;
  auto Argv = ::CommandLineToArgvW(GetCommandLineW(), &numArgc);
  if (Argv) {
    if (numArgc >= 2 && bela::PathExists(Argv[1])) {
      /// ---> todo set value
      ResolveLink(Argv[1]);
    }
    LocalFree(Argv);
  }
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
  RefreshFont(hFont, dpiY);
  renderTarget->SetDpi(static_cast<float>(dpiX), static_cast<float>(dpiX));
  UpdateWidgetPos(wUrl);
  UpdateWidgetPos(wPicker);
  if (wCharacteristics.Alived()) {
    UpdateWidgetPos(wCharacteristics);
  }
  if (wDepends.Alived()) {
    UpdateWidgetPos(wDepends);
  }
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
  ResolveLink(buf);
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
  ResolveLink(*file);
  return S_OK;
}

inline std::wstring BaseName(std::wstring_view sv) {
  if (sv.empty()) {
    return L".";
  }
  auto pos = sv.find_last_not_of(L"\\/");
  if (pos == std::wstring_view::npos) {
    return L"/";
  }
  sv.remove_suffix(sv.size() - pos - 1);
  pos = sv.find_last_of(L"\\/");
  if (pos != std::wstring_view::npos) {
    sv.remove_prefix(pos + 1);
  }
  if (sv.empty()) {
    return L".";
  }
  return std::wstring(sv);
}

bool Window::ResolveLink(std::wstring_view file) {
  //
  std::lock_guard<std::mutex> lock(mtx);
  tables.Clear();
  wCharacteristics.Destroy();
  wDepends.Destroy();
  ::InvalidateRect(hWnd, NULL, TRUE);
  if (file.empty()) {
    return false;
  }
  bela::error_code ec;
  auto link = caelum::ResolveLink(file, ec);
  if (!link) {
    bela::BelaMessageBox(
        hWnd, L"unable lookup file link",
        bela::StringCat(L"File: ", BaseName(file), L"\nError: ", ec.message)
            .data(),
        nullptr, bela::mbs_t::FATAL);
    return false;
  }
  ::SetWindowTextW(wUrl.hWnd, link->data());
  auto ret = InquisitivePE();
  ::InvalidateRect(hWnd, NULL, TRUE);
  return ret;
}

} // namespace caelum