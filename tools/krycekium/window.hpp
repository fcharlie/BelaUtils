////
#include "krycekium.hpp"
#define _ATL_NO_AUTOMATIC_NAMESPACE
#include <windowsx.h>
#include <atlbase.h>
#include <atlctl.h>
#include <atlwin.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <string>
#include <string_view>
#include <vector>
#include <baulkenv.hpp>
//
#include "executor.hpp"

#ifndef SYSCOMMAND_ID_HANDLER
#define SYSCOMMAND_ID_HANDLER(id, func)                                                                                \
  if (uMsg == WM_SYSCOMMAND && id == LOWORD(wParam)) {                                                                 \
    bHandled = TRUE;                                                                                                   \
    lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled);                                            \
    if (bHandled)                                                                                                      \
      return TRUE;                                                                                                     \
  }
#endif

namespace krycekium {
namespace ui {
enum WidgetNumber : ptrdiff_t {
  about = 1001, //
  source = 1002,
  folder = 1003,
  picker = 1004,
  pickerdir = 1005,
  progress = 1006,
  execute = 1007,
  cancel = 1008
};
}

using namespace ATL;

struct Label {
  Label() = default;
  Label(LONG left, LONG top, LONG right, LONG bottom, std::wstring_view sv = L"") {
    layout.left = left;
    layout.top = top;
    layout.right = right;
    layout.bottom = bottom;
    text = sv;
  }
  Label &operator=(const Label &o) {
    text = o.text;
    layout.left = o.layout.left;
    layout.top = o.layout.top;
    layout.right = o.layout.right;
    layout.bottom = o.layout.bottom;
    return *this;
  }
  bool empty() const { return text.empty(); }
  const wchar_t *data() const { return text.data(); };
  UINT32 length() const { return static_cast<UINT32>(text.size()); }
  D2D1_RECT_F FR() const {
    return D2D1::RectF(static_cast<float>(layout.left), static_cast<float>(layout.top),
                       static_cast<float>(layout.right), static_cast<float>(layout.bottom));
  }
  RECT layout;
  std::wstring text;
};

struct Widget {
  HWND hWnd{nullptr};
  int X{0};
  int Y{0};
  int W{0};
  int H{0};
  void Visible(BOOL v) { EnableWindow(hWnd, v); }
  bool IsVisible() const { return IsWindowVisible(hWnd) == TRUE; }
  std::wstring Content() {
    auto n = GetWindowTextLengthW(hWnd);
    if (n <= 0) {
      return L"";
    }
    std::wstring str;
    str.resize(n + 1);
    auto k = GetWindowTextW(hWnd, str.data(), n + 1);
    str.resize(k);
    return str;
  }
  void Content(std::wstring_view text) {
    //
    ::SetWindowTextW(hWnd, text.data());
  }
};

constexpr const wchar_t *WindowName = L"Krycekium.Window";
using WindowTraits = CWinTraits<WS_OVERLAPPEDWINDOW, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE>;
// Per-Monitor DPI Aware

class Window : public CWindowImpl<Window, CWindow, WindowTraits> {
public:
  DECLARE_WND_CLASS(WindowName)
  BEGIN_MSG_MAP(Window)
  MESSAGE_HANDLER(WM_CREATE, OnCreate)
  MESSAGE_HANDLER(WM_CLOSE, OnClose)
  MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
  MESSAGE_HANDLER(WM_SIZE, OnSize)
  MESSAGE_HANDLER(WM_PAINT, OnPaint)
  MESSAGE_HANDLER(WM_DPICHANGED, OnDpiChanged)
  MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnDisplayChange)
  MESSAGE_HANDLER(WM_DROPFILES, OnDropfiles)
  // User defined message
  MESSAGE_HANDLER(WM_EXECUTOR_NOTIFY, OnExecutorNotify)
  SYSCOMMAND_ID_HANDLER(ui::about, OnKrycekiumAbout)
  COMMAND_ID_HANDLER(ui::picker, OnSourceView)
  COMMAND_ID_HANDLER(ui::pickerdir, OnFolderView)
  COMMAND_ID_HANDLER(ui::execute, OnExecuteTask)
  COMMAND_ID_HANDLER(ui::cancel, OnCancelTask)
  END_MSG_MAP()
  LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnDpiChanged(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
  LRESULT OnDropfiles(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
  LRESULT OnExecutorNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  // Command Handle
  LRESULT OnKrycekiumAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
  LRESULT OnSourceView(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
  LRESULT OnFolderView(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
  LRESULT OnExecuteTask(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
  LRESULT OnCancelTask(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);

private:
  // UI function
  HRESULT CreateDeviceIndependentResources();
  HRESULT CreateDeviceResources();
  void DiscardDeviceResources();
  HRESULT OnRender();
  HRESULT RefreshDxFont();
  HRESULT RefreshGdiFont();
  //  Feature
  bool MakeWidget(Widget &w, LPCWSTR cn, LPCWSTR text, DWORD ds, int x, int y, int cx, int cy, ptrdiff_t id) {
    constexpr const auto wndex = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY;
    w.X = x;
    w.Y = y;
    w.W = cx;
    w.H = cy;
    w.hWnd = ::CreateWindowExW(wndex, cn, text, ds, MulDiv(w.X, dpiX, 96), MulDiv(w.Y, dpiX, 96), MulDiv(w.W, dpiX, 96),
                               MulDiv(w.H, dpiX, 96), m_hWnd, reinterpret_cast<HMENU>(id), hInst, nullptr);
    if (w.hWnd == nullptr) {
      return false;
    }
    ::SendMessageW(w.hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }
  bool MakeEdit(Widget &w, LPCWSTR text, int x, int y, int cx, int cy, ptrdiff_t id) {
    constexpr const auto eex =
        WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE;
    constexpr const auto es = WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
    w.X = x;
    w.Y = y;
    w.W = cx;
    w.H = cy;
    w.hWnd =
        ::CreateWindowExW(eex, WC_EDITW, text, es, MulDiv(w.X, dpiX, 96), MulDiv(w.Y, dpiX, 96), MulDiv(w.W, dpiX, 96),
                          MulDiv(w.H, dpiX, 96), m_hWnd, reinterpret_cast<HMENU>(id), hInst, nullptr);
    if (w.hWnd == nullptr) {
      return false;
    }
    ::SendMessageW(w.hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }

  bool UpdateWidgetPos(Widget &w) {
    if (w.hWnd == nullptr) {
      return false;
    }
    ::SetWindowPos(w.hWnd, NULL, MulDiv(w.X, dpiX, 96), MulDiv(w.Y, dpiX, 96), MulDiv(w.W, dpiX, 96),
                   MulDiv(w.H, dpiX, 96), SWP_NOZORDER | SWP_NOACTIVATE);
    ::SendMessageW(w.hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return true;
  }

public:
  Window();
  ~Window();
  bool MakeWindow();

private:
  HINSTANCE hInst;
  ID2D1Factory *factory{nullptr};
  ID2D1HwndRenderTarget *renderTarget{nullptr};
  ID2D1SolidColorBrush *lineBrush{nullptr};
  ID2D1SolidColorBrush *textBrush{nullptr};
  IDWriteFactory *dwFactory{nullptr};
  IDWriteTextFormat *dwFormat{nullptr};
  std::vector<Label> labels;
  Label notice;
  Widget wSource;
  Widget wFolder;
  Widget wPicker;
  Widget wPickerDir;
  Widget wProgress;
  Widget wExecute;
  Widget wCancel;
  belautils::BaulkEnv baulkenv;
  HFONT hFont{nullptr}; // GDI font
  UINT32 dpiX{96};
  //
  Executor executor;
};
} // namespace krycekium
