//
#include "caelum.hpp"
#include <bela/picker.hpp>
#include <CommCtrl.h>
#include <VersionHelpers.h>
#include <bela/picker.hpp>
#include "window.hpp"

class dotcom_initializer {
public:
  dotcom_initializer() {
    if (FAILED(CoInitialize(nullptr))) {
      MessageBoxW(nullptr, L"CoInitialize()", L"CoInitialize() failed", MB_OK | MB_ICONERROR);
    }
  }
  ~dotcom_initializer() { CoUninitialize(); }
};

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  dotcom_initializer di;
  if (!IsWindows10OrGreater()) {
    bela::BelaMessageBox(nullptr, L"You need at least Windows 10",
                         L"Please upgrade Your OS to Windows 10", nullptr, bela::mbs_t::FATAL);
    return -1;
  }
  HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
  INITCOMMONCONTROLSEX info = {sizeof(INITCOMMONCONTROLSEX),
                               ICC_TREEVIEW_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES};
  InitCommonControlsEx(&info);
  caelum::Window window;
  if (!window.MakeWindow()) {
    auto ec = bela::make_system_error_code();
    bela::BelaMessageBox(nullptr, L"unable create window", ec.data(), nullptr, bela::mbs_t::FATAL);
    return 1;
  }
  window.RunMessageLoop();
  return 0;
}