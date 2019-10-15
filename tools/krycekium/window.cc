///
#include "window.hpp"
// PE BaseAddress
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace krycekium {

template <typename T> void Free(T *t) {
  if (*t != nullptr) {
    (*t)->Release();
    *t = nullptr;
  }
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
  return S_OK;
}
LRESULT Window::OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam,
                                BOOL &bHandled) {
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

} // namespace krycekium