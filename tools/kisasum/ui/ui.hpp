#ifndef KISASUM_UI_HPP
#define KISASUM_UI_HPP
#pragma once
#include <bela/base.hpp>
#include <atlbase.h>
#include <atlctl.h>
#include <atlwin.h>
#include <atomic>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <memory>
#include <string>
#include <vector>
#include <wincodec.h>
#include "kisasum.hpp"

namespace kisasum::ui {

struct WindowSettings {
  /// color
  std::uint32_t panelcolor{0x00BFFF};
  std::uint32_t contentcolor{0xffffff};
  std::uint32_t textcolor{0x000000};
  std::uint32_t labelcolor{0x000000};
  std::wstring title{L"Kisasum Immersive"};
  std::wstring font{L"Segoe UI"};
  bool Update();
  bool Flush();
};

//// what's fuck HEX color and COLORREF color, red <-- --> blue
//// this is LE CPU, BE CPU don't call
inline COLORREF calcLuminance(UINT32 cr) {
  int r = (cr & 0xff0000) >> 16;
  int g = (cr & 0xff00) >> 8;
  int b = (cr & 0xff);
  return RGB(r, g, b);
}

struct D2D1Checkbox {
  RECT Layout;
  std::wstring Text;
  bool IsChecked;
};

#define KISASUM_WINDOW_NAME L"Kisasum.UI"

#ifndef SYSCOMMAND_ID_HANDLER
#define SYSCOMMAND_ID_HANDLER(id, func)                                                            \
  if (uMsg == WM_SYSCOMMAND && id == LOWORD(wParam)) {                                             \
    bHandled = TRUE;                                                                               \
    lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled);                        \
    if (bHandled)                                                                                  \
      return TRUE;                                                                                 \
  }
#endif

#define NEON_WINDOW_CLASSSTYLE                                                                     \
  WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS & ~WS_MAXIMIZEBOX
typedef CWinTraits<NEON_WINDOW_CLASSSTYLE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE> CMetroWindowTraits;

class CDPI;
/// Awesome
class Window : public CWindowImpl<Window, CWindow, CMetroWindowTraits> {
private:
  ID2D1Factory *pFactory;
  ID2D1HwndRenderTarget *pHwndRenderTarget;
  ID2D1SolidColorBrush *AppPageBackgroundThemeBrush;
  ID2D1SolidColorBrush *AppPageTextBrush;
  IDWriteFactory *pWriteFactory;
  // IDWriteTextFormat* pTitleWriteTextFormat;//Alignment left
  // IDWriteTextFormat* pButtonWriteTextFormat; /// Alignment center
  IDWriteTextFormat *pLabelWriteTextFormat; /// Alignment left
  HRESULT CreateDeviceIndependentResources();
  HRESULT Initialize();
  HRESULT CreateDeviceResources();
  void DiscardDeviceResources();
  HRESULT OnRender();
  D2D1_SIZE_U CalculateD2DWindowSize();
  void OnResize(UINT width, UINT height);
  /////////// self control handle
  LRESULT Filesum(std::wstring_view file);
  void UpdateTitle(std::wstring_view title_);
  bool CopyToClipboard(std::wstring_view text);
  bool UpdateTheme();
  std::unique_ptr<CDPI> dpi_;
  HFONT hFont{nullptr};
  HWND hCombo{nullptr};
  HWND hOpenButton{nullptr};
  HWND hClearButton{nullptr};
  HWND hCheck{nullptr};
  HBRUSH hBrush{nullptr};
  WindowSettings ws;
  std::wstring filetext;
  std::wstring sizetext;
  std::wstring hash;
  std::atomic_uint32_t progress{0};
  std::atomic_bool locked{false};
  int dpiX;
  int dpiY;
  std::uint32_t height;
  std::uint32_t width;
  std::uint32_t areaheight;
  std::uint32_t keywidth{90};
  std::uint32_t lineheight{20};
  bool showerror{false};

public:
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window();
  ~Window();
  WindowSettings &Settings() { return ws; }
  LRESULT InitializeWindow();
  DECLARE_WND_CLASS(KISASUM_WINDOW_NAME)
  BEGIN_MSG_MAP(MetroWindow)
  MESSAGE_HANDLER(WM_CREATE, OnCreate)
  MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnColorStatic)
  MESSAGE_HANDLER(WM_KEYDOWN, OnKeydown);
  MESSAGE_HANDLER(WM_SIZE, OnSize)
  MESSAGE_HANDLER(WM_PAINT, OnPaint)
  MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnDisplayChange)
  MESSAGE_HANDLER(WM_DPICHANGED, OnDpiChanged)
  MESSAGE_HANDLER(WM_DROPFILES, OnDropfiles)
  MESSAGE_HANDLER(WM_CLOSE, OnClose)
  MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
  MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
  COMMAND_ID_HANDLER(IDC_CLEAR_BUTTON, OnContentClear)
  COMMAND_ID_HANDLER(IDC_FILEOPEN_BUTTON, OnOpenFile)
  COMMAND_ID_HANDLER(IDM_CONTEXT_COPY, OnCopy)
  SYSCOMMAND_ID_HANDLER(IDM_CHANGE_THEME, OnTheme)
  SYSCOMMAND_ID_HANDLER(IDM_APP_INFO, OnAbout)
  END_MSG_MAP()
  LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnDpiChanged(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
  LRESULT OnKeydown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
  LRESULT OnDropfiles(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
  LRESULT OnColorStatic(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnColorButton(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
  LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
  LRESULT OnContentClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
  LRESULT OnOpenFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
  LRESULT OnTheme(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
  LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
  LRESULT OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
};
} // namespace kisasum::ui
#endif