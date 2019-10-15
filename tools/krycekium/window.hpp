////
#include "krycekium.hpp"
#define _ATL_NO_AUTOMATIC_NAMESPACE
#include <atlbase.h>
#include <atlctl.h>
#include <atlwin.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

namespace krycekium {
using namespace ATL;
constexpr const wchar_t *WindowName = L"Krycekium.Window";
using WindowTraits =
    CWinTraits<WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN |
                   WS_CLIPSIBLINGS & ~WS_MAXIMIZEBOX,
               WS_EX_APPWINDOW | WS_EX_WINDOWEDGE>;

class Window : public CWindowImpl<Window, CWindow, WindowTraits> {
public:
  Window();
  ~Window();
  bool WindowInitialize();
  bool Show();

protected:
  DECLARE_WND_CLASS(WindowName)
  BEGIN_MSG_MAP(Window)
  MESSAGE_HANDLER(WM_CREATE, OnCreate)
  MESSAGE_HANDLER(WM_CLOSE, OnClose)
  MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
  //   MESSAGE_HANDLER(WM_SIZE, OnSize)
  //   MESSAGE_HANDLER(WM_PAINT, OnPaint)
  //   MESSAGE_HANDLER(WM_DPICHANGED, OnDpiChanged)
  //   MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnDisplayChange)
  //   MESSAGE_HANDLER(WM_DROPFILES, OnDropfiles)
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
  LRESULT OnExecutorNotify(UINT nMsg, WPARAM wParam, LPARAM lParam,
                           BOOL &bHandle);
  LRESULT OnExecutorProgress(UINT nMsg, WPARAM wParam, LPARAM lParam,
                             BOOL &bHandle);

private:
  HINSTANCE hInst;
};
} // namespace krycekium