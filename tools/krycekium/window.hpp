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
constexpr const wchar_t *WindowName = L"Krycekium.Window";
using WindowTraits =
    CWinTraits<WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN |
                   WS_CLIPSIBLINGS & ~WS_MAXIMIZEBOX,
               WS_EX_APPWINDOW | WS_EX_WINDOWEDGE>;

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
  //   SYSCOMMAND_ID_HANDLER(IDM_KRYCEKIUM_ABOUT, OnKrycekiumAbout)
  //   COMMAND_ID_HANDLER(IDC_PACKAGE_VIEW_BUTTON, OnDiscoverPackage)
  //   COMMAND_ID_HANDLER(IDC_FOLDER_URI_BUTTON, OnDiscoverFolder)
  //   COMMAND_ID_HANDLER(IDC_OPTION_BUTTON_OK, OnStartTask)
  //   COMMAND_ID_HANDLER(IDC_OPTION_BUTTON_CANCEL, OnCancelTask)
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
  // UI function
  HRESULT CreateDeviceIndependentResources();
  HRESULT Initialize();
  HRESULT CreateDeviceResources();
  void DiscardDeviceResources();
  HRESULT OnRender();
  D2D1_SIZE_U CalculateD2DWindowSize();
  void OnResize(UINT32 width, UINT32 height);
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
};
} // namespace krycekium
