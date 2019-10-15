///
#include "window.hpp"
// PE BaseAddress
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace krycekium {
Window::Window() {
  //
  hInst = reinterpret_cast<HINSTANCE>(&__ImageBase);
}

Window::~Window() {
  //
}

bool Window::WindowInitialize() {
  //
  return true;
}

LRESULT Window::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam,
                         BOOL &bHandle) {
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