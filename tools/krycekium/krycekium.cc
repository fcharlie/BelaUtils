/////
#include "krycekium.hpp"
#include "window.hpp"
#include <bela/picker.hpp>
#include <VersionHelpers.h>
// Main

class dotcom_initializer {
public:
  dotcom_initializer() {
    if (FAILED(CoInitialize(nullptr))) {
      MessageBoxW(nullptr, L"CoInitialize()", L"CoInitialize() failed",
                  MB_OK | MB_ICONERROR);
    }
  }
  ~dotcom_initializer() { CoUninitialize(); }
};

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  dotcom_initializer di;
  if (!IsWindows10OrGreater()) {
    bela::BelaMessageBox(nullptr, L"You need at least Windows 10",
                         L"Please upgrade Your OS to Windows 10", nullptr,
                         bela::mbs_t::FATAL);
    return -1;
  }
  HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
  INITCOMMONCONTROLSEX info = {sizeof(INITCOMMONCONTROLSEX),
                               ICC_TREEVIEW_CLASSES | ICC_COOL_CLASSES |
                                   ICC_LISTVIEW_CLASSES};
  InitCommonControlsEx(&info);
  krycekium::Window window;
  if (!window.MakeWindow()) {
    auto ec = bela::make_system_error_code();
    bela::BelaMessageBox(nullptr, L"unable create window", ec.message.data(),
                         nullptr, bela::mbs_t::FATAL);
    return 1;
  }
  window.ShowWindow(SW_SHOW);
  window.UpdateWindow();
  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  return 0;
}