//
#include "ui.hpp"
//
#include <CommCtrl.h>
#include <Mmsystem.h>
#include <PathCch.h>
#include <Prsht.h>
#include <ShellScalingApi.h>
#include <Shellapi.h>
#include <Shlwapi.h>
#include <Windowsx.h>
#include <algorithm>
#include <cctype>
#include <commdlg.h>
#include <cstdlib>
#include <filesystem>
#include <ppltasks.h>
#include <bela/picker.hpp>
#include <bela/fmt.hpp>
#include "../../../lib/hashlib/sumizer.hpp"

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

namespace kisasum::ui {

template <class Interface> inline void SafeRelease(Interface **ppInterfaceToRelease) {
  if (*ppInterfaceToRelease != NULL) {
    (*ppInterfaceToRelease)->Release();

    (*ppInterfaceToRelease) = NULL;
  }
}

class CDPI {
public:
  CDPI() {
    m_nScaleFactor = 0;
    m_nScaleFactorSDA = 0;
    m_Awareness = PROCESS_DPI_UNAWARE;
  }

  int Scale(int x) {
    // DPI Unaware:  Return the input value with no scaling.
    // These apps are always virtualized to 96 DPI and scaled by the system for
    // the DPI of the monitor where shown.
    if (m_Awareness == PROCESS_DPI_UNAWARE) {
      return x;
    }

    // System DPI Aware:  Return the input value scaled by the factor determined
    // by the system DPI when the app was launched. These apps render themselves
    // according to the DPI of the display where they are launched, and they
    // expect that scaling to remain constant for all displays on the system.
    // These apps are scaled up or down when moved to a display with a different
    // DPI from the system DPI.
    if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
      return MulDiv(x, m_nScaleFactorSDA, 100);
    }

    // Per-Monitor DPI Aware:  Return the input value scaled by the factor for
    // the display which contains most of the window. These apps render
    // themselves for any DPI, and re-render when the DPI changes (as indicated
    // by the WM_DPICHANGED window message).
    return MulDiv(x, m_nScaleFactor, 100);
  }

  UINT GetScale() {
    if (m_Awareness == PROCESS_DPI_UNAWARE) {
      return 100;
    }

    if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
      return m_nScaleFactorSDA;
    }

    return m_nScaleFactor;
  }

  void SetScale(__in UINT iDPI) {
    m_nScaleFactor = MulDiv(iDPI, 100, 96);
    if (m_nScaleFactorSDA == 0) {
      m_nScaleFactorSDA = m_nScaleFactor; // Save the first scale factor, which
                                          // is all that SDA apps know about
    }
    return;
  }

  PROCESS_DPI_AWARENESS GetAwareness() {
    HANDLE hProcess;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
    GetProcessDpiAwareness(hProcess, &m_Awareness);
    return m_Awareness;
  }

  void SetAwareness(PROCESS_DPI_AWARENESS awareness) {
    HRESULT hr = E_FAIL;
    hr = SetProcessDpiAwareness(awareness);
    auto l = E_INVALIDARG;
    if (hr == S_OK) {
      m_Awareness = awareness;
    }
  }

  // Scale rectangle from raw pixels to relative pixels.
  void ScaleRect(__inout RECT *pRect) {
    pRect->left = Scale(pRect->left);
    pRect->right = Scale(pRect->right);
    pRect->top = Scale(pRect->top);
    pRect->bottom = Scale(pRect->bottom);
  }

  // Scale Point from raw pixels to relative pixels.
  void ScalePoint(__inout POINT *pPoint) {
    pPoint->x = Scale(pPoint->x);
    pPoint->y = Scale(pPoint->y);
  }

private:
  UINT m_nScaleFactor;
  UINT m_nScaleFactorSDA;
  PROCESS_DPI_AWARENESS m_Awareness;
};

Window::Window()
    : pFactory(nullptr), AppPageBackgroundThemeBrush(nullptr), AppPageTextBrush(nullptr),
      renderTarget(nullptr), pWriteFactory(nullptr), lableTextFormat(nullptr) {
  dpi_ = std::make_unique<CDPI>();
  dpi_->GetAwareness();
  // dpi_->SetAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
}

Window::~Window() {
  SafeRelease(&lableTextFormat);
  SafeRelease(&pWriteFactory);
  SafeRelease(&AppPageBackgroundThemeBrush);
  SafeRelease(&AppPageTextBrush);
  SafeRelease(&renderTarget);
  SafeRelease(&pFactory);
  if (hFont != nullptr) {
    DeleteFont(hFont);
  }
  if (hBrush != nullptr) {
    DeleteObject(hBrush);
  }
}
#define WS_NORESIZEWINDOW                                                                          \
  (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_MINIMIZEBOX)

LRESULT Window::InitializeWindow() {
  HMONITOR hMonitor;
  POINT pt;
  UINT dpix = 0, dpiy = 0;
  HRESULT hr = E_FAIL;

  // Get the DPI for the main monitor, and set the scaling factor
  pt.x = 1;
  pt.y = 1;
  hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
  hr = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy);

  if (hr != S_OK) {
    ::MessageBox(NULL, (LPCWSTR)L"GetDpiForMonitor failed", (LPCWSTR)L"Notification", MB_OK);
    return FALSE;
  }
  dpi_->SetScale(dpix);
  /// create D2D1
  if (Initialize() != S_OK) {
    ::MessageBoxW(nullptr, L"Initialize() failed", L"Fatal error", MB_OK | MB_ICONSTOP);
    std::terminate();
    return S_FALSE;
  }
  RECT layout = {CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT + dpi_->Scale(700),
                 CW_USEDEFAULT + dpi_->Scale(370)};
  width = 700;
  height = 370;
  areaheight = 250;
  Create(nullptr, layout, ws.title.data(), WS_NORESIZEWINDOW, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
  return S_OK;
}

/////
HRESULT Window::CreateDeviceIndependentResources() {
  HRESULT hr = S_OK;
  hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
  if (SUCCEEDED(hr)) {
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                             reinterpret_cast<IUnknown **>(&pWriteFactory));
    if (SUCCEEDED(hr)) {
      hr = pWriteFactory->CreateTextFormat(ws.font.c_str(), NULL, DWRITE_FONT_WEIGHT_NORMAL,
                                           DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                           16.0f, L"zh-CN", &lableTextFormat);
    }
  }
  return hr;
}
HRESULT Window::Initialize() {
  auto hr = CreateDeviceIndependentResources();
  // FLOAT dpiX, dpiY;
  if (hr == S_OK) {
    pFactory->ReloadSystemMetrics();
    // pFactory->GetDesktopDpi(&dpiX, &dpiY);
  }
  return hr;
}
HRESULT Window::CreateDeviceResources() {
  HRESULT hr = S_OK;

  if (!renderTarget) {
    RECT rc;
    ::GetClientRect(m_hWnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
    hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                          D2D1::HwndRenderTargetProperties(m_hWnd, size),
                                          &renderTarget);
    if (SUCCEEDED(hr)) {
      ////
      hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF((UINT32)ws.panelcolor),
                                               &AppPageBackgroundThemeBrush);
    }
    if (SUCCEEDED(hr)) {
      hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(ws.labelcolor), &AppPageTextBrush);
    }
  }
  return hr;
}
void Window::DiscardDeviceResources() {
  SafeRelease(&AppPageBackgroundThemeBrush);
  SafeRelease(&AppPageTextBrush);
}

template <typename A, typename B, typename C, typename D>
auto RectF(A left, B top, C right, D bottom) {
  return D2D1::RectF(static_cast<FLOAT>(left), static_cast<FLOAT>(top), static_cast<FLOAT>(right),
                     static_cast<FLOAT>(bottom));
}

HRESULT Window::OnRender() {
  auto hr = CreateDeviceResources();
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  auto size = renderTarget->GetSize();
  renderTarget->BeginDraw();
  renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
  renderTarget->Clear(D2D1::ColorF(ws.contentcolor));
  renderTarget->FillRectangle(RectF(0, areaheight, size.width, size.height),
                              AppPageBackgroundThemeBrush);
  constexpr std::wstring_view uppercase = L"Uppercase";
  renderTarget->DrawTextW(uppercase.data(), static_cast<UINT32>(uppercase.size()), lableTextFormat,
                          RectF(40, areaheight + 28.0f, 200.0f, areaheight + 50.0f),
                          AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                          DWRITE_MEASURING_MODE_NATURAL);
  if (!filetext.empty()) {
    constexpr std::wstring_view name = L"Name:";
    renderTarget->DrawTextW(name.data(), static_cast<UINT32>(name.size()), lableTextFormat,
                            RectF(20, 5.0f, keywidth, lineheight), AppPageTextBrush,
                            D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
    renderTarget->DrawTextW(filetext.data(), static_cast<UINT32>(filetext.size()), lableTextFormat,
                            RectF(keywidth, 5.0f, size.width - 20, lineheight), AppPageTextBrush,
                            D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
  }
  if (!sizetext.empty()) {
    constexpr std::wstring_view Size = L"Size:";
    renderTarget->DrawTextW(Size.data(), static_cast<UINT32>(Size.size()), lableTextFormat,
                            RectF(20, lineheight + 5.0f, keywidth, lineheight * 2),
                            AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
    renderTarget->DrawTextW(sizetext.data(), static_cast<UINT32>(sizetext.size()), lableTextFormat,
                            RectF(keywidth, lineheight + 5.0f, size.width - 20, lineheight * 2),
                            AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
  }
  if (!hash.empty()) {
    constexpr std::wstring_view Hash = L"Hash:";
    renderTarget->DrawTextW(Hash.data(), static_cast<UINT32>(Hash.size()), lableTextFormat,
                            RectF(20, lineheight * 2 + 5.0f, keywidth, lineheight * 3),
                            AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
    renderTarget->DrawTextW(hash.data(), static_cast<UINT32>(hash.size()), lableTextFormat,
                            RectF(keywidth, lineheight * 2 + 5.0f, size.width - 20, lineheight * 5),
                            AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
  }
  //// write progress 100 %
  if (progress == 100) {
    constexpr std::wstring_view Hundred = L"💯";
    renderTarget->DrawTextW(Hundred.data(), static_cast<UINT32>(Hundred.size()), lableTextFormat,
                            RectF(160.0f, areaheight + 28.0f, 250.0f, areaheight + 50.5f),
                            AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
  } else if (progress > 0 && progress < 100) {
    auto progressText = bela::StringCat(static_cast<uint32_t>(progress), L"%");
    renderTarget->DrawTextW(
        progressText.data(), static_cast<uint32_t>(progressText.size()), lableTextFormat,
        RectF(160.0f, areaheight + 28.0f, 250.0f, areaheight + 50.5f), AppPageTextBrush,
        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT, DWRITE_MEASURING_MODE_NATURAL);
  } else if (showerror) {
    constexpr std::wstring_view fatal = L"😟";
    renderTarget->DrawTextW(fatal.data(), static_cast<UINT32>(fatal.size()), lableTextFormat,
                            RectF(160.0f, areaheight + 28.0f, 250.0f, areaheight + 50.5f),
                            AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
  } else {
    constexpr std::wstring_view text = L"\u2764Kisasum";
    renderTarget->DrawTextW(text.data(), (UINT32)text.size(), lableTextFormat,
                            RectF(160.0f, areaheight + 28.0f, 250.0f, areaheight + 50.5f),
                            AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
  }
  hr = renderTarget->EndDraw();
  if (hr == D2DERR_RECREATE_TARGET) {
    hr = S_OK;
    DiscardDeviceResources();
    //::InvalidateRect(m_hWnd, nullptr, FALSE);
  }
  return hr;
}
D2D1_SIZE_U Window::CalculateD2DWindowSize() {
  RECT rc;
  ::GetClientRect(m_hWnd, &rc);

  D2D1_SIZE_U d2dWindowSize = {0};
  d2dWindowSize.width = rc.right;
  d2dWindowSize.height = rc.bottom;

  return d2dWindowSize;
}

void Window::OnResize(UINT width, UINT height) {
  if (renderTarget) {
    renderTarget->Resize(D2D1::SizeU(width, height));
  }
}

void Window::UpdateTitle(std::wstring_view title_) {
  std::wstring xtitle = ws.title;
  if (!title_.empty()) {
    xtitle.append(L" - ").append(title_);
  }
  SetWindowTextW(xtitle.data());
}

bool Window::UpdateTheme() {
  if (hBrush) {
    DeleteObject(hBrush);
  }
  hBrush = CreateSolidBrush(calcLuminance(ws.panelcolor));
  SafeRelease(&AppPageBackgroundThemeBrush);
  auto hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF((UINT32)ws.panelcolor),
                                                &AppPageBackgroundThemeBrush);
  ::InvalidateRect(hCheck, nullptr, TRUE);
  InvalidateRect(nullptr, TRUE);
  return true;
}
constexpr const std::wstring_view HashAlgorithm[] = {
    L"BLAKE3",   L"SHA224",   L"SHA256",   L"SHA384",  L"SHA512",  L"SHA3-224",
    L"SHA3-256", L"SHA3-384", L"SHA3-512", L"BLAKE2s", L"BLAKE2b", L"KangarooTwelve", //
};

//////////////////////////
inline bool InitializeComboHash(HWND hWnd) {
  for (auto c : HashAlgorithm) {
    ::SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)c.data());
  }
  ::SendMessage(hWnd, CB_SETCURSEL, 0, 0);
  return true;
}

//// Window  style-ex
#define KWS_WINDOWEX WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY
//// Edit style
#define KWS_EDIT                                                                                   \
  WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL
/// push button style
#define KWS_BUTTON BS_PUSHBUTTON | BS_FLAT | BS_TEXT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE
/// checkbox style
#define KWS_CHECKBOX                                                                               \
  BS_FLAT | BS_TEXT | BS_CHECKBOX | BS_AUTOCHECKBOX | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE
/// combobox style
#define KWS_COMBOBOX                                                                               \
  WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_HASSTRINGS
#define DEFAULT_PADDING96 20

LRESULT Window::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  dpiX = GetDpiForWindow(m_hWnd);
  dpiY = dpiX;
  dpi_->SetScale(dpiX);
  HICON hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_KISASUM));
  SetIcon(hIcon, TRUE);
  ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
  ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
  ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
  ::DragAcceptFiles(m_hWnd, TRUE);
  hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  LOGFONT logFont = {0};
  GetObjectW(hFont, sizeof(logFont), &logFont);
  DeleteObject(hFont);
  hFont = NULL;
  logFont.lfHeight = dpi_->Scale(DEFAULT_PADDING96);
  logFont.lfWeight = FW_NORMAL;
  wcscpy_s(logFont.lfFaceName, L"Segoe UI");

  hFont = CreateFontIndirectW(&logFont);
  auto LambdaCreateWindow = [&](LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X,
                                int Y, int nWidth, int nHeight, HMENU hMenu) -> HWND {
    auto hw = CreateWindowExW(KWS_WINDOWEX, lpClassName, lpWindowName, dwStyle, dpi_->Scale(X),
                              dpi_->Scale(Y), dpi_->Scale(nWidth), dpi_->Scale(nHeight), m_hWnd,
                              hMenu, HINST_THISCOMPONENT, nullptr);
    if (hw) {
      ::SendMessageW(hw, WM_SETFONT, (WPARAM)hFont, lParam);
    }
    return hw;
  };
  auto LambdaCreateWindowEdge = [&](LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X,
                                    int Y, int nWidth, int nHeight, HMENU hMenu) -> HWND {
    auto hw = CreateWindowExW(KWS_WINDOWEX | WS_EX_CLIENTEDGE, lpClassName, lpWindowName, dwStyle,
                              dpi_->Scale(X), dpi_->Scale(Y), dpi_->Scale(nWidth),
                              dpi_->Scale(nHeight), m_hWnd, hMenu, HINST_THISCOMPONENT, nullptr);
    if (hw) {
      ::SendMessageW(hw, WM_SETFONT, (WPARAM)hFont, lParam);
    }
    return hw;
  };
  RECT rect;
  GetWindowRect(&rect);

  hCheck = LambdaCreateWindow(WC_BUTTONW, L"", KWS_CHECKBOX, 20, areaheight + 30, 20, 20, nullptr);

  hCombo = LambdaCreateWindow(WC_COMBOBOXW, L"", KWS_COMBOBOX, width - 420, areaheight + 25, 120,
                              27, nullptr);

  hOpenButton = LambdaCreateWindow(WC_BUTTONW, L"Clear", KWS_BUTTON, width - 290, areaheight + 25,
                                   120, 27, HMENU(IDC_CLEAR_BUTTON));

  hOpenButton = LambdaCreateWindow(WC_BUTTONW, L"Open", KWS_BUTTON, width - 160, areaheight + 25,
                                   120, 27, HMENU(IDC_FILEOPEN_BUTTON));

  InitializeComboHash(hCombo);
  hBrush = CreateSolidBrush(calcLuminance(ws.panelcolor));
  HMENU hSystemMenu = ::GetSystemMenu(m_hWnd, FALSE);
  InsertMenuW(hSystemMenu, SC_CLOSE, MF_ENABLED, IDM_CHANGE_THEME, L"Change Panel Color");
  InsertMenuW(hSystemMenu, SC_CLOSE, MF_ENABLED, IDM_APP_INFO, L"About Kisasum Immersive\tAlt+F1");
  return S_OK;
}

LRESULT Window::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  if (hBrush) {
    DeleteObject(hBrush);
  }
  PostQuitMessage(0);
  return S_OK;
}

LRESULT Window::OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  ::DestroyWindow(m_hWnd);
  return S_OK;
}

LRESULT Window::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  UINT width = LOWORD(lParam);
  UINT height = HIWORD(lParam);
  OnResize(width, height);
  return S_OK;
}

LRESULT Window::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  OnRender();
  ValidateRect(NULL);
  return S_OK;
}

LRESULT Window::OnDpiChanged(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  HMONITOR hMonitor;
  POINT pt;
  UINT dpix = 0, dpiy = 0;
  HRESULT hr = E_FAIL;

  // Get the DPI for the main monitor, and set the scaling factor
  pt.x = 1;
  pt.y = 1;
  hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
  hr = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy);

  if (hr != S_OK) {
    ::MessageBox(NULL, (LPCWSTR)L"GetDpiForMonitor failed", (LPCWSTR)L"Notification", MB_OK);
    return FALSE;
  }
  dpi_->SetScale(dpix);
  RECT *const prcNewWindow = (RECT *)lParam;
  ::SetWindowPos(m_hWnd, NULL, prcNewWindow->left, prcNewWindow->top,
                 dpi_->Scale(prcNewWindow->right - prcNewWindow->left),
                 dpi_->Scale(prcNewWindow->bottom - prcNewWindow->top),
                 SWP_NOZORDER | SWP_NOACTIVATE);
  LOGFONTW logFont = {0};
  GetObjectW(hFont, sizeof(logFont), &logFont);
  DeleteObject(hFont);
  hFont = nullptr;
  logFont.lfHeight = dpi_->Scale(DEFAULT_PADDING96);
  logFont.lfWeight = FW_NORMAL;
  wcscpy_s(logFont.lfFaceName, L"Segoe UI");
  hFont = CreateFontIndirectW(&logFont);
  auto UpdateWindowPos = [&](HWND hWnd) {
    RECT rect;
    ::GetClientRect(hWnd, &rect);
    ::SetWindowPos(hWnd, NULL, dpi_->Scale(rect.left), dpi_->Scale(rect.top),
                   dpi_->Scale(rect.right - rect.left), dpi_->Scale(rect.bottom - rect.top),
                   SWP_NOZORDER | SWP_NOACTIVATE);
    ::SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, lParam);
  };
  UpdateWindowPos(hCombo);
  UpdateWindowPos(hOpenButton);
  UpdateWindowPos(hClearButton);
  UpdateWindowPos(hCheck);
  return S_OK;
}

LRESULT Window::OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
  ::InvalidateRect(m_hWnd, NULL, FALSE);
  return S_OK;
}

bool IsKeyPressed(UINT nVirtKey) { return GetKeyState(nVirtKey) < 0 ? true : false; }

LRESULT Window::OnKeydown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
  bool fCtrlDown = IsKeyPressed(VK_CONTROL);
  switch (wParam) {
  case 'c':
  case 'C':
    CopyToClipboard(hash);
    MessageBeep(MB_OK);
    return S_OK;
  default:
    return 0;
  }
  return S_OK;
}

LRESULT Window::OnColorStatic(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  HDC hdc = (HDC)wParam;
  auto hControl = reinterpret_cast<HWND>(lParam);
  if (hControl == hCheck) {
    SetBkMode(hdc, TRANSPARENT);
    // SetBkColor(hdc, RGB(255, 255, 255));
    SetTextColor(hdc, calcLuminance(ws.labelcolor));
    return (LRESULT)((HBRUSH)hBrush);
  }
  return ::DefWindowProc(m_hWnd, nMsg, wParam, lParam);
}

LRESULT Window::OnColorButton(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle) {
  HDC hdc = (HDC)wParam;
  SetTextColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);
  return reinterpret_cast<LRESULT>(hBrush);
}

LRESULT Window::OnRButtonUp(UINT, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
  POINT pt;
  GetCursorPos(&pt);
  auto ptc = pt;
  ScreenToClient(&pt);
  if (pt.y >= (LONG)areaheight) {
    //// NOT Area
    return S_OK;
  }
  HMENU hMenu = ::LoadMenuW(GetModuleHandle(nullptr), MAKEINTRESOURCEW(IDM_MAIN_CONTEXT));
  HMENU hPopu = GetSubMenu(hMenu, 0);
  auto resourceIcon2Bitmap = [](int id) -> HBITMAP {
    HICON icon = LoadIconW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(id));
    HDC hDc = ::GetDC(NULL);
    HDC hMemDc = CreateCompatibleDC(hDc);
    auto hBitmap = CreateCompatibleBitmap(hDc, 16, 16);
    SelectObject(hMemDc, hBitmap);
    HBRUSH hBrush = GetSysColorBrush(COLOR_MENU);
    DrawIconEx(hMemDc, 0, 0, icon, 16, 16, 0, hBrush, DI_NORMAL);
    DeleteObject(hBrush);
    DeleteDC(hMemDc);
    ::ReleaseDC(NULL, hDc);
    DestroyIcon(icon);
    return hBitmap;
  };
  HBITMAP hBitmap = nullptr;
  if (hash.empty()) {
    EnableMenuItem(hPopu, IDM_CONTEXT_COPY, MF_DISABLED);
  } else {
    EnableMenuItem(hPopu, IDM_CONTEXT_COPY, MF_ENABLED);
    hBitmap = resourceIcon2Bitmap(IDI_ICON_COPY);
    if (hBitmap) {
      SetMenuItemBitmaps(hPopu, IDM_CONTEXT_COPY, MF_BYCOMMAND, hBitmap, hBitmap);
    }
  }

  ::TrackPopupMenuEx(hPopu, TPM_RIGHTBUTTON, ptc.x, ptc.y, m_hWnd, nullptr);
  ::DestroyMenu(hMenu);
  if (hBitmap) {
    DeleteObject(hBitmap);
  }
  return S_OK;
}

LRESULT Window::OnContentClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled) {
  if (!locked) {
    filetext.clear();
    sizetext.clear();
    hash.clear();
    showerror = false;
    progress = 0;
    UpdateTitle(L"");
    InvalidateRect(nullptr, FALSE);
  }
  return S_OK;
}

std::wstring EncodeSize(int64_t size) {
  constexpr auto GB = 1024 * 1024 * 1024ULL;
  constexpr auto MB = 1024 * 1024ULL;
  constexpr auto KB = 1024ULL;
  if (size > GB) {
    return bela::StrFormat(L"%.2f GB", (double)size / GB);
  }
  if (size > MB) {
    return bela::StrFormat(L"%.2f MB", (double)size / MB);
  }
  if (size > KB) {
    return bela::StrFormat(L"%.2f KB", (double)size / KB);
  }
  return bela::StringCat(size, L" bytes");
}

LRESULT Window::Filesum(std::wstring_view file) {
  bool except = false;
  if (!locked.compare_exchange_strong(except, true)) {
    return false;
  }
  auto i = ComboBox_GetCurSel(hCombo);
  if (i < 0 || static_cast<size_t>(i) >= std::size(HashAlgorithm)) {
    locked = false;
    return S_FALSE;
  }
  auto hashName = HashAlgorithm[i];
  filetext.clear();
  hash.clear();
  sizetext.clear();
  std::error_code e;
  std::filesystem::path p(file);
  auto title = bela::StringCat(L"(", hashName, L") ", p.filename().wstring());
  UpdateTitle(title);
  auto ha = belautils::lookup_algorithm(hashName);
  if (ha == belautils::algorithm::NONE) {
    locked = false;
    return S_FALSE;
  }
  Concurrency::create_task([this, p, ha]() -> bool {
    auto closer = bela::finally([this] { this->locked = false; });
    auto file = p.wstring();
    auto hFile = CreateFileW(file.data(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
      return false;
    }

    LARGE_INTEGER li;
    GetFileSizeEx(hFile, &li);
    filetext = file;
    if (file.size() > 64) {
      filetext = p.filename().wstring();
      if (filetext.size() > 64) {
        filetext.resize(61);
        filetext.append(L"...");
      }
    }
    sizetext = EncodeSize(li.QuadPart);
    InvalidateRect(nullptr);
    DWORD dwRead;
    int64_t total = 0;
    uint32_t pg = 0;
    constexpr DWORD buflen = 8192;
    unsigned char buffer[8192];
    auto sum = belautils::make_sumizer(ha);
    if (!sum) {
      return false;
    }
    for (;;) {
      if (!ReadFile(hFile, buffer, buflen, &dwRead, nullptr)) {
        break;
      }
      sum->Update(buffer, static_cast<size_t>(dwRead));
      total += dwRead;
      auto N = (uint32_t)(total * 100 / li.QuadPart);
      progress = (uint32_t)N;
      /// when number is modify, Flush Window
      if (pg != N) {
        pg = (uint32_t)N;
        InvalidateRect(nullptr, FALSE);
      }
      if (dwRead < buflen) {
        break;
      }
    }
    CloseHandle(hFile);
    hash.clear();
    bool ucase = (Button_GetCheck(hCheck) == BST_CHECKED);
    sum->Final(hash, ucase);
    return true;
  }).then([this](bool result) {
    if (!result) {
      showerror = true;
    }
    InvalidateRect(nullptr, FALSE);
    locked = false;
  });
  return S_OK;
}

LRESULT Window::OnDropfiles(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
  HDROP hDrop = (HDROP)wParam;
  WCHAR file[32267];
  UINT nfilecounts = DragQueryFileW(hDrop, 0, file, sizeof(file));
  DragFinish(hDrop);
  if (nfilecounts == 0) {
    return S_OK;
  }
  return Filesum(file);
}

LRESULT Window::OnOpenFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled) {
  typedef COMDLG_FILTERSPEC FilterSpec;
  const FilterSpec filterSpec[] = {{L"All Files (*.*)", L"*.*"}};
  auto resp = bela::FilePicker(m_hWnd, L"Open File", filterSpec);
  if (!resp) {
    return S_OK;
  }
  return Filesum(*resp);
}

static COLORREF CustColors[] = {
    RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
    RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
    RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
    RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
};

LRESULT Window::OnTheme(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled) {
  auto color = calcLuminance(ws.panelcolor);
  CHOOSECOLORW co;
  ZeroMemory(&co, sizeof(co));
  co.lStructSize = sizeof(CHOOSECOLOR);
  co.hwndOwner = m_hWnd;
  co.lpCustColors = (LPDWORD)CustColors;
  co.rgbResult = calcLuminance(ws.panelcolor);
  co.lCustData = 0;
  co.lpTemplateName = nullptr;
  co.lpfnHook = nullptr;
  co.Flags = CC_FULLOPEN | CC_RGBINIT;
  if (ChooseColorW(&co)) {
    auto r = GetRValue(co.rgbResult);
    auto g = GetGValue(co.rgbResult);
    auto b = GetBValue(co.rgbResult);
    ws.panelcolor = (r << 16) + (g << 8) + b;
    UpdateTheme();
    ws.Flush();
  }
  return S_OK;
}

LRESULT Window::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled) {
  bela::BelaMessageBox(m_hWnd, L"About Kisasum Hash Utilities", BELAUTILS_APPVERSION,
                       BELAUTILS_APPLINK, bela::mbs_t::ABOUT);
  return S_OK;
}

bool Window::CopyToClipboard(std::wstring_view text) {
  if (text.empty()) {
    return false;
  }
  if (!OpenClipboard()) {
    return false;
  }
  if (!EmptyClipboard()) {
    CloseClipboard();
    return false;
  }
  HGLOBAL hgl = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t));
  if (hgl == nullptr) {
    CloseClipboard();
    return false;
  }
  LPWSTR ptr = (LPWSTR)GlobalLock(hgl);
  memcpy(ptr, text.data(), (text.size() + 1) * sizeof(wchar_t));
  SetClipboardData(CF_UNICODETEXT, hgl);
  CloseClipboard();
  return true;
}

LRESULT Window::OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled) {
  CopyToClipboard(hash);
  return S_OK;
}
} // namespace kisasum::ui