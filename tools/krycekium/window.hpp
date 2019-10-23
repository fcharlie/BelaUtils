////
#include "krycekium.hpp"
#define _ATL_NO_AUTOMATIC_NAMESPACE
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

// UI Index
#define IDM_KRYCEKIUM_ABOUT 1001
#define IDE_SOURCE_URI 1002
#define IDE_FOLDER_URI 1003
#define IDB_SOURCE_VIEW 1004
#define IDB_FOLDER_VIEW 1005
#define IDP_PROGRESS_STATE 1006
#define IDB_EXECUTE_TASK 1007
#define IDB_CANCEL_TASK 1008

#ifndef SYSCOMMAND_ID_HANDLER
#define SYSCOMMAND_ID_HANDLER(id, func)                                        \
  if (uMsg == WM_SYSCOMMAND && id == LOWORD(wParam)) {                         \
    bHandled = TRUE;                                                           \
    lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled);    \
    if (bHandled)                                                              \
      return TRUE;                                                             \
  }
#endif

namespace krycekium {
using namespace ATL;

struct Label {
  Label() = default;
  Label(LONG left, LONG top, LONG right, LONG bottom,
        std::wstring_view sv = L"") {
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
    return D2D1::RectF(
        static_cast<float>(layout.left), static_cast<float>(layout.top),
        static_cast<float>(layout.right), static_cast<float>(layout.bottom));
  }
  RECT layout;
  std::wstring text;
};

constexpr const wchar_t *WindowName = L"Krycekium.Window";
using WindowTraits =
    CWinTraits<WS_OVERLAPPEDWINDOW, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE>;
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
  MESSAGE_HANDLER(WM_EXECUTOR_PROGRESS, OnExecutorProgress)
  SYSCOMMAND_ID_HANDLER(IDM_KRYCEKIUM_ABOUT, OnKrycekiumAbout)
  COMMAND_ID_HANDLER(IDB_SOURCE_VIEW, OnSourceView)
  COMMAND_ID_HANDLER(IDB_FOLDER_VIEW, OnFolderView)
  COMMAND_ID_HANDLER(IDB_EXECUTE_TASK, OnExecuteTask)
  COMMAND_ID_HANDLER(IDB_CANCEL_TASK, OnCancelTask)
  END_MSG_MAP()
  LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnDpiChanged(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam,
                          BOOL &bHandled);
  LRESULT OnDropfiles(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
  LRESULT OnExecutorNotify(UINT nMsg, WPARAM wParam, LPARAM lParam,
                           BOOL &bHandle);
  LRESULT OnExecutorProgress(UINT nMsg, WPARAM wParam, LPARAM lParam,
                             BOOL &bHandle);
  // Command Handle
  LRESULT OnKrycekiumAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                           BOOL &bHandled);
  LRESULT OnSourceView(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                       BOOL &bHandled);
  LRESULT OnFolderView(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                       BOOL &bHandled);
  LRESULT OnExecuteTask(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                        BOOL &bHandled);
  LRESULT OnCancelTask(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                       BOOL &bHandled);
  // UI function
  HRESULT CreateDeviceIndependentResources();
  HRESULT CreateDeviceResources();
  void DiscardDeviceResources();
  HRESULT OnRender();
  HRESULT RefreshDxFont();
  HRESULT RefreshGdiFont();
  //  Feature
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
  HWND hSource;
  HWND hFolder;
  HWND hPicker;
  HWND hDirPicker;
  HWND hProgress;
  HWND hExecute;
  HWND hCancel;
  HFONT hFont{nullptr}; // GDI font
  UINT32 dpiX{96};
  std::vector<Label> labels;
  Label notice;
};
} // namespace krycekium
