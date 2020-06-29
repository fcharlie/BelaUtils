//
#include <bela/base.hpp>
#include <CommCtrl.h>
#include <commdlg.h>
#include <regex>
#include <wchar.h>
#include "ui.hpp"
#include "kisasum.hpp"

class DotComInitialize {
public:
  DotComInitialize() { CoInitialize(NULL); }
  ~DotComInitialize() { CoUninitialize(); }
};

int WindowLoop() {
  INITCOMMONCONTROLSEX info = {sizeof(INITCOMMONCONTROLSEX),
                               ICC_TREEVIEW_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES};
  InitCommonControlsEx(&info);
  kisasum::ui::Window window;
  MSG msg;
  window.Settings().Update();
  if (window.InitializeWindow() != S_OK) {
    return 1;
  }
  window.ShowWindow(SW_SHOW);
  window.UpdateWindow();
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  DotComInitialize dot;
  HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
  return WindowLoop();
}
