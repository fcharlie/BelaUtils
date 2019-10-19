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
// https://blogs.windows.com/windowsdeveloper/2017/05/19/improving-high-dpi-experience-gdi-based-desktop-apps/#Uwv9gY1SvpbgQ4dK.97
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
}

bool Window::WindowInitialize() {
  //
  return true;
}

bool Window::RefreshWindow() {
  auto dpiX = GetDpiForWindow(m_hWnd);
  if (dpiX == 0) {
    return false;
  }

  return true;
}

LRESULT Window::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam,
                         BOOL &bHandle) {
  auto dpiX = GetDpiForWindow(m_hWnd);
  // Resize window with dpi

  return S_OK;
}
LRESULT Window::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam,
                          BOOL &bHandle) {
  return S_OK;
}
LRESULT Window::OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam,
                        BOOL &bHandle) {
  return S_OK;
}

LRESULT Window::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  return S_OK;
}
LRESULT Window::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam,
                        BOOL &bHandle) {
  return S_OK;
}

LRESULT Window::OnDpiChanged(UINT nMsg, WPARAM wParam, LPARAM lParam,
                             BOOL &bHandle) {
  //
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

HRESULT Window::CreateDeviceIndependentResources() {
  //
  return S_OK;
}

HRESULT Window::Initialize() {
  //
  return S_OK;
}

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

D2D1_SIZE_U Window::CalculateD2DWindowSize() {
  RECT rc;
  ::GetClientRect(m_hWnd, &rc);
  return D2D1_SIZE_U{static_cast<UINT32>(rc.right),
                     static_cast<UINT32>(rc.bottom)};
}

void Window::OnResize(UINT width, UINT height) {
  //
}

} // namespace krycekium
