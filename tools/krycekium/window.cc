///
#include <bela/picker.hpp>
#include <bela/strcat.hpp>
#include <bela/stdwriter.hpp>
#include <shellapi.h>
#include "window.hpp"
#include "kmutils.hpp"
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

template <typename T> void FreeObj(T *t) {
  if (*t != nullptr) {
    DeleteObject(*t);
    *t = nullptr;
  }
}

static inline int Year() {
  SYSTEMTIME stime;
  GetSystemTime(&stime);
  return stime.wYear;
}

Window::Window() {
  hInst = reinterpret_cast<HINSTANCE>(&__ImageBase);
  // create
  labels.emplace_back(30, 50, 120, 75, L"Package:");
  labels.emplace_back(30, 100, 120, 125, L"Folder:");
  notice = Label(125, 345, 600, 370);
  notice.text = bela::StringCat(L"\xD83D\xDE0B \x2764 Copyright \x0A9 ", Year(),
                                L", Force Charlie. All Rights Reserved.");
}

Window::~Window() {
  Free(&renderTarget);
  Free(&factory);
  Free(&lineBrush);
  Free(&textBrush);
  Free(&dwFactory);
  Free(&dwFormat);
  FreeObj(&hFont);
}

// https://docs.microsoft.com/en-us/windows/win32/api/_hidpi/

bool Window::MakeWindow() {
  if (CreateDeviceIndependentResources() < 0) {
    return false;
  }
  const auto noresizewindow = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                              WS_CLIPCHILDREN | WS_MINIMIZEBOX;
  RECT rect = {100, 100, 800, 540};
  Create(nullptr, rect, L"Krycekium", noresizewindow,
         WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
  return executor.InitializeExecutor();
}

// Safely reset fonts.
HRESULT Window::RefreshGdiFont() {
  auto dfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  if (dfont == nullptr) {
    return false;
  }
  LOGFONT logFont = {0};
  auto dwsize = GetObjectW(dfont, sizeof(logFont), &logFont);
  DeleteObject(dfont);
  if (dwsize == 0) {
    return false;
  }
  logFont.lfHeight = MulDiv(20, dpiX, 96);
  logFont.lfWeight = FW_NORMAL;
  wcscpy_s(logFont.lfFaceName, L"Segoe UI");
  auto newFont = CreateFontIndirectW(&logFont);
  if (newFont == nullptr) {
    return false;
  }
  FreeObj(&hFont);
  hFont = newFont;
  return true;
}

// https://docs.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritefactory-createtextformat
// FontSize
HRESULT Window::RefreshDxFont() {
  Free(&dwFormat);
  return dwFactory->CreateTextFormat(
      L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"zh-CN", &dwFormat);
}

HRESULT Window::CreateDeviceIndependentResources() {
  auto hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  return DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                             __uuidof(IDWriteFactory),
                             reinterpret_cast<IUnknown **>(&dwFactory));
}

//
HRESULT Window::CreateDeviceResources() {
  if (renderTarget != nullptr) {
    return S_OK;
  }
  RECT rc;
  ::GetClientRect(m_hWnd, &rc);
  auto size = D2D1::SizeU(static_cast<UINT>(rc.right - rc.left),
                          static_cast<UINT>(rc.bottom - rc.top));
  auto hr = factory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(m_hWnd, size), &renderTarget);
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  renderTarget->SetDpi(static_cast<float>(dpiX), static_cast<float>(dpiX));
  // https://hashtagcolor.com/
  hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),
                                           &textBrush);
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  return renderTarget->CreateSolidColorBrush(
      D2D1::ColorF(D2D1::ColorF::DarkSalmon), &lineBrush);
}

void Window::DiscardDeviceResources() {
  Free(&renderTarget); // free renderTarget will recreate
  Free(&lineBrush);
  Free(&textBrush);
}

HRESULT Window::OnRender() {
  auto hr = CreateDeviceResources();
  if (!SUCCEEDED(hr)) {
    return hr;
  }
  if ((renderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED) != 0) {
    return S_OK;
  }
  renderTarget->BeginDraw();
  renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
  renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::WhiteSmoke, 1.0f));
  auto size = renderTarget->GetSize();
  renderTarget->DrawRectangle(D2D1::RectF(20, 10, size.width - 20, 155),
                              lineBrush, 1.0);
  renderTarget->DrawRectangle(
      D2D1::RectF(20, 155, size.width - 20, size.height - 20), lineBrush, 1.0);
  // D2D1_DRAW_TEXT_OPTIONS_NONE
  for (auto &label : labels) {
    if (label.empty()) {
      continue;
    }
    renderTarget->DrawTextW(label.data(), label.length(), dwFormat, label.FR(),
                            textBrush, D2D1_DRAW_TEXT_OPTIONS_NONE,
                            DWRITE_MEASURING_MODE_NATURAL);
  }
  if (!notice.empty()) {
    renderTarget->DrawTextW(notice.data(), notice.length(), dwFormat,
                            notice.FR(), textBrush,
                            D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                            DWRITE_MEASURING_MODE_NATURAL);
  }
  hr = renderTarget->EndDraw();
  if (hr == D2DERR_RECREATE_TARGET) {
    DiscardDeviceResources();
  }
  return hr;
}

LRESULT Window::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam,
                         BOOL &bHandle) {
  auto cs = reinterpret_cast<CREATESTRUCTW const *>(lParam);
  dpiX = GetDpiForWindow(m_hWnd);
  // Refresh Window use real DPI
  RECT rect;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
  int cx = rect.right - rect.left;
  auto w = MulDiv(700, dpiX, 96);
  ::SetWindowPos(m_hWnd, nullptr, (cx - w) / 2, MulDiv(100, dpiX, 96), w,
                 MulDiv(440, dpiX, 96), SWP_NOZORDER | SWP_NOACTIVATE);
  // Enable Drag files
  ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
  ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
  ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
  ::DragAcceptFiles(m_hWnd, TRUE);
  // Draw Window ICON
  auto hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_KRYCEKIUM_ICON));
  SetIcon(hIcon, TRUE);
  RefreshDxFont();
  RefreshGdiFont();
  if (hFont == nullptr) {
    hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  }
  // child window styles
  constexpr const auto bs =
      BS_PUSHBUTTON | BS_TEXT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE;
  constexpr const auto ps = WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE;

  MakeEdit(wSource, L"", 125, 50, 420, 27, ui::source);
  MakeEdit(wFolder, L"", 125, 100, 420, 27, ui::folder);
  MakeWidget(wPicker, WC_BUTTONW, L"View...", bs, 560, 50, 90, 27, ui::picker);
  MakeWidget(wPickerDir, WC_BUTTONW, L"Folder...", bs, 560, 100, 90, 27,
             ui::pickerdir);
  MakeWidget(wProgress, PROGRESS_CLASSW, L"", ps, 125, 180, 420, 27,
             ui::progress);
  MakeWidget(wExecute, WC_BUTTONW, L"Start", bs, 125, 270, 200, 30,
             ui::execute);
  MakeWidget(wCancel, WC_BUTTONW, L"Cancel", bs, 340, 270, 205, 30, ui::cancel);
  wCancel.Visible(FALSE);

  HMENU hSystemMenu = ::GetSystemMenu(m_hWnd, FALSE);
  InsertMenuW(hSystemMenu, SC_CLOSE, MF_ENABLED,
              static_cast<UINT_PTR>(ui::about), L"About Krycekium\tAlt+F1");
  //
  int numArgc = 0;
  auto Argv = ::CommandLineToArgvW(GetCommandLineW(), &numArgc);
  if (Argv) {
    if (numArgc >= 2 && PathFileExistsW(Argv[1])) {
      std::wstring_view sv(Argv[1]);
      if (IsSuffixEnabled(sv)) {
        wSource.Content(sv);
        auto sameFolder = krycekium::SameFolder(sv);
        if (sameFolder) {
          wFolder.Content(*sameFolder);
        }
      }
    }
    LocalFree(Argv);
  }
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
  if (renderTarget != nullptr) {
    renderTarget->Resize(D2D1::SizeU(width, height));
  }
  return S_OK;
}

LRESULT Window::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam,
                        BOOL &bHandle) {
  PAINTSTRUCT ps;
  BeginPaint(&ps);
  OnRender();
  EndPaint(&ps);
  return S_OK;
}

// https://docs.microsoft.com/zh-cn/windows/win32/hidpi/wm-dpichanged todo
LRESULT Window::OnDpiChanged(UINT nMsg, WPARAM wParam, LPARAM lParam,
                             BOOL &bHandle) {
  dpiX = static_cast<UINT32>(LOWORD(wParam));
  auto prcNewWindow = reinterpret_cast<RECT *const>(lParam);
  // resize window with new DPI
  ::SetWindowPos(m_hWnd, nullptr, prcNewWindow->left, prcNewWindow->top,
                 prcNewWindow->right - prcNewWindow->left,
                 prcNewWindow->bottom - prcNewWindow->top,
                 SWP_NOZORDER | SWP_NOACTIVATE);
  renderTarget->SetDpi(static_cast<float>(dpiX), static_cast<float>(dpiX));
  RefreshGdiFont();
  UpdateWidgetPos(wSource);
  UpdateWidgetPos(wFolder);
  UpdateWidgetPos(wPicker);
  UpdateWidgetPos(wPickerDir);
  UpdateWidgetPos(wProgress);
  UpdateWidgetPos(wExecute);
  UpdateWidgetPos(wCancel);
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
  ::SetWindowTextW(m_hWnd, L"Krycekium");
  const LPCWSTR PackageSubffix[] = {L".msi", L".msp"};
  HDROP hDrop = (HDROP)wParam;
  UINT nfilecounts = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
  WCHAR dn[8192] = {0};
  if (nfilecounts != 1) {
    DragFinish(hDrop);
    return S_OK;
  }
  auto n = DragQueryFileW(hDrop, 0, dn, 8192);
  if (n == 0) {
    DragFinish(hDrop);
    return S_OK;
  }
  std::wstring_view sv(dn, n);
  if (IsSuffixEnabled(sv)) {
    wSource.Content(sv);
    auto sameFolder = krycekium::SameFolder(sv);
    if (sameFolder) {
      wFolder.Content(*sameFolder);
    }
  }
  DragFinish(hDrop);
  return S_OK;
}

// User defined message handler
LRESULT Window::OnExecutorNotify(UINT nMsg, WPARAM wParam, LPARAM lParam,
                                 BOOL &bHandle) {
  auto status = static_cast<krycekium::Status>(wParam);
  switch (status) {
  case krycekium::Status::None:
    break;
  case krycekium::Status::Completed:
    wExecute.Visible(TRUE);
    wPicker.Visible(TRUE);
    wPickerDir.Visible(TRUE);
    wCancel.Visible(FALSE);
    ::MessageBeep(IDOK);
    ::SetWindowTextW(m_hWnd, L"Krycekium (Completed)");
    break;
  case krycekium::Status::Failure: {
    auto ec = static_cast<DWORD>(lParam);
    auto msg = bela::resolve_system_error_message(ec);
    bela::BelaMessageBox(m_hWnd, L"Unable to extract msi package", msg.data(),
                         nullptr, bela::mbs_t::FATAL);
    ::MessageBeep(MB_ICONERROR);
    ::SetWindowTextW(m_hWnd, L"Krycekium (Failure)");
    wExecute.Visible(TRUE);
    wPicker.Visible(TRUE);
    wPickerDir.Visible(TRUE);
    wCancel.Visible(FALSE);
  } break;
  default:
    break;
  }
  return S_OK;
}

// Command Handle
LRESULT Window::OnKrycekiumAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                 BOOL &bHandled) {
  bela::BelaMessageBox(m_hWnd, L"About Krycekium MSI Unpacker", FULL_COPYRIGHT,
                       KRYCEKIUM_APPLINK);
  return S_OK;
}

//

LRESULT Window::OnSourceView(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                             BOOL &bHandled) {
  ::SetWindowTextW(m_hWnd, L"Krycekium");
  constexpr const bela::filter_t filters[] = {
      {L"Windows Installer Package (*.msi;*.msp)", L"*.msi;*.msp"},
      {L"All Files (*.*)", L"*.*"}};
  auto file = bela::FilePicker(m_hWnd, L"Windows Installer Package", filters);
  if (file) {
    wSource.Content(*file);
    auto sameFolder = krycekium::SameFolder(*file);
    if (sameFolder) {
      wFolder.Content(*sameFolder);
    }
  }
  return S_OK;
}

LRESULT Window::OnFolderView(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                             BOOL &bHandled) {
  ::SetWindowTextW(m_hWnd, L"Krycekium");
  auto folder = bela::FolderPicker(m_hWnd, L"Open Folder");
  if (folder) {
    wFolder.Content(*folder);
  }
  return S_OK;
}

LRESULT Window::OnExecuteTask(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                              BOOL &bHandled) {
  auto msi = wSource.Content();
  if (msi.empty()) {
    bela::BelaMessageBox(
        m_hWnd, L"MSI package not input",
        L"Please enter the path of the msi package to be extracted", nullptr,
        bela::mbs_t::FATAL);
    return S_OK;
  }
  if (!bela::PathExists(msi)) {
    bela::BelaMessageBox(m_hWnd, L"MSI package not found", msi.data(), nullptr,
                         bela::mbs_t::FATAL);
    return S_OK;
  }
  auto folder = wFolder.Content();
  if (folder.empty()) {
    bela::BelaMessageBox(m_hWnd, L"Destination not input",
                         L"Please set destination", nullptr,
                         bela::mbs_t::FATAL);
    return S_OK;
  }
  // check directory is exists and empty
  if (bela::PathExists(folder, bela::FileAttribute::Dir)) {
    if (!krycekium::FolderIsEmpty(folder)) {
      if (::MessageBoxW(
              m_hWnd,
              L"Destination is not empty, whether to overwrite the file",
              L"Destination is not empty",
              MB_OKCANCEL | MB_ICONWARNING) != MB_OK) {
        return S_OK;
      }
    }
  } else {
    if (CreateDirectoryW(folder.data(), nullptr) != TRUE) {
      auto ec = bela::make_system_error_code();
      bela::BelaMessageBox(m_hWnd, L"Destination cannot be created", ec.data(),
                           nullptr, bela::mbs_t::FATAL);
      return S_OK;
    }
  }
  if (!executor.PushEvent(msi, folder, m_hWnd, wProgress.hWnd)) {
    return S_OK;
  }
  wPicker.Visible(FALSE);
  wPickerDir.Visible(FALSE);
  wExecute.Visible(FALSE);
  wCancel.Visible(TRUE);
  ::SetWindowTextW(m_hWnd, L"Krycekium (Running...)");
  return S_OK;
}

LRESULT Window::OnCancelTask(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                             BOOL &bHandled) {
  executor.Cancel();
  ::SetWindowTextW(m_hWnd, L"Krycekium (Canceled)");
  return S_OK;
}

} // namespace krycekium
