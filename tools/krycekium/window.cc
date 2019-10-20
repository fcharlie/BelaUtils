///
#include <bela/picker.hpp>
#include <bela/strcat.hpp>
#include "window.hpp"
#include "resource.h"

// PE BaseAddress
extern "C" IMAGE_DOS_HEADER __ImageBase;

// DPI details
// LoadIconWithScaleDown
// https://github.com/tringi/win32-dpi
// https://github.com/microsoft/Windows-classic-samples/tree/master/Samples/DPIAwarenessPerWindow
// https://blogs.windows.com/windowsdeveloper/2017/05/19/improving-high-dpi-experience-gdi-based-desktop-apps/
// https://docs.microsoft.com/zh-cn/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
/*
#define DPI_AWARENESS_CONTEXT_UNAWARE              ((DPI_AWARENESS_CONTEXT)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         ((DPI_AWARENESS_CONTEXT)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#define DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED    ((DPI_AWARENESS_CONTEXT)-5)
*/

namespace krycekium {

template <typename T> void Free(T *t) {
  if (*t != nullptr) {
    (*t)->Release();
    *t = nullptr;
  }
}

bool FolderIsEmpty(const std::wstring &dir) {
  auto xdir = bela::StringCat(dir, L"\\*.*");
  WIN32_FIND_DATA fdata;
  auto hFind = FindFirstFileW(xdir.data(), &fdata);
  if (hFind == INVALID_HANDLE_VALUE) {
    return true;
  }
  constexpr std::wstring_view dot = L".";
  constexpr std::wstring_view dotdot = L"..";
  for (;;) {
    if (dotdot != fdata.cFileName && dot != fdata.cFileName) {
      FindClose(hFind);
      return false;
    }
    if (FindNextFile(hFind, &fdata) != TRUE) {
      break;
    }
  }
  FindClose(hFind);
  return true;
}

Window::Window() {
  //
  hInst = reinterpret_cast<HINSTANCE>(&__ImageBase);
}

Window::~Window() {
  Free(&renderTarget);
  Free(&factory);
  Free(&lineBrush);
  Free(&dwFactory);
  Free(&dwFormat);
}

bool Window::MakeWindow() {
  //
  return true;
}

LRESULT Window::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam,
                         BOOL &bHandle) {
  auto cs = reinterpret_cast<CREATESTRUCTW const *>(lParam);
  dpiX = GetDpiForWindow(m_hWnd);
  // Resize window with dpi

  return S_OK;
}

LRESULT Window::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam,
                          BOOL &bHandle) {
  PostQuitMessage(0);
  return S_OK;
}

LRESULT Window::OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam,
                        BOOL &bHandle) {
  ::DestroyWindow(m_hWnd);
  return S_OK;
}

LRESULT Window::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  UINT width = LOWORD(lParam);
  UINT height = HIWORD(lParam);
  OnResize(width, height);
  return S_OK;
}

LRESULT Window::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam,
                        BOOL &bHandle) {
  OnRender();
  ValidateRect(NULL);
  return S_OK;
}

// https://docs.microsoft.com/zh-cn/windows/win32/hidpi/wm-dpichanged todo
LRESULT Window::OnDpiChanged(UINT nMsg, WPARAM wParam, LPARAM lParam,
                             BOOL &bHandle) {
  dpiX = static_cast<UINT32>(wParam);
  auto prcNewWindow = reinterpret_cast<RECT *const>(lParam);
  // resize window with new DPI
  ::SetWindowPos(m_hWnd, nullptr, prcNewWindow->left, prcNewWindow->top,
                 prcNewWindow->right - prcNewWindow->left,
                 prcNewWindow->bottom - prcNewWindow->top,
                 SWP_NOZORDER | SWP_NOACTIVATE);
  if (!SUCCEEDED(RefreshDxFont())) {
    //
  }
  //::SetWindowPos()
  return S_OK;
}

LRESULT Window::OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam,
                                BOOL &bHandled) {
  ::InvalidateRect(m_hWnd, NULL, FALSE);
  return S_OK;
}

LRESULT Window::OnDropfiles(UINT nMsg, WPARAM wParam, LPARAM lParam,
                            BOOL &bHandled) {

  return S_OK;
}

// User defined message handler
LRESULT Window::OnExecutorNotify(UINT nMsg, WPARAM wParam, LPARAM lParam,
                                 BOOL &bHandle) {
  return S_OK;
}

LRESULT Window::OnExecutorProgress(UINT nMsg, WPARAM wParam, LPARAM lParam,
                                   BOOL &bHandle) {
  return S_OK;
}

// Command Handle
LRESULT Window::OnKrycekiumAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                 BOOL &bHandled) {
  bela::BelaMessageBox(m_hWnd, L"About Krycekium MSI Unpacker", FULL_COPYRIGHT,
                       KRYCEKIUM_APPLINK);
  return S_OK;
}

///

HRESULT Window::RefreshDxFont() {
  Free(&dwFormat);
  auto fontsize = MulDiv(12 * 96 / 72, dpiX, 96);
  return dwFactory->CreateTextFormat(
      L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, static_cast<float>(fontsize), L"zh-CN",
      &dwFormat);
}

HRESULT Window::CreateDeviceIndependentResources() {
  auto hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                           reinterpret_cast<IUnknown **>(&dwFactory));
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  return RefreshDxFont();
}

HRESULT Window::Initialize() {
  //
  return S_OK;
}

HRESULT Window::CreateDeviceResources() {
  if (renderTarget != nullptr) {
    return S_OK;
  }
  RECT rc;
  ::GetClientRect(m_hWnd, &rc);
  auto size = D2D1::SizeU(static_cast<UINT>(rc.right - rc.left),
                          static_cast<UINT>(rc.bottom = rc.top));
  auto hr = factory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(m_hWnd, size), &renderTarget);
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  return S_OK;
}

void Window::DiscardDeviceResources() {
  //
}

HRESULT Window::OnRender() {
  //
  return S_OK;
}

D2D1_SIZE_U Window::CalculateD2DWindowSize() {
  RECT rc;
  ::GetClientRect(m_hWnd, &rc);
  return D2D1::SizeU(static_cast<UINT32>(rc.right),
                     static_cast<UINT32>(rc.bottom));
}

void Window::OnResize(UINT32 width, UINT32 height) {
  //
}

} // namespace krycekium
