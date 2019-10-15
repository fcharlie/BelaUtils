// executor.cc
#include "krycekium.hpp"
#include "executor.hpp"
#include <Msi.h>

namespace krycekium {
// sender
class Sender {
public:
  Sender() = default;
  Sender(const Sender &) = delete;
  Sender &operator=(const Sender &) = delete;

private:
};

// install_ui_callback
INT WINAPI install_ui_callback(LPVOID ctx, UINT iMessageType,
                               LPCWSTR szMessage) {
  auto executor = reinterpret_cast<Executor *>(ctx);

  return 0;
}
// https://docs.microsoft.com/zh-cn/windows/win32/msi/handling-progress-messages-using-msisetexternalui
struct ProgressFileds {
  int field[4];
};

int FGetInteger(wchar_t *&rpch) {
  wchar_t *pchPrev = rpch;
  while (*rpch && *rpch != ' ') {
    rpch++;
  }
  *rpch = '\0';
  int i = _wtoi(pchPrev);
  return i;
}

//
//  FUNCTION: ParseProgressString(LPSTR sz)
//
//  PURPOSE:  Parses the progress data message sent to the INSTALLUI_HANDLER
//  callback
//
//  COMMENTS: Assumes correct syntax.
//
bool ParseProgressString(LPWSTR sz, ProgressFileds &fileds) {
  wchar_t *pch = sz;
  if (0 == *pch) {
    return false; // no msg
  }

  while (*pch != 0) {
    char chField = *pch++;
    pch++; // for ':'
    pch++; // for sp
    switch (chField) {
    case '1': // field 1
      // progress message type
      if (0 == isdigit(*pch)) {
        return false; // blank record
      }
      fileds.field[0] = *pch++ - '0';
      break;
    case '2': // field 2
      fileds.field[1] = FGetInteger(pch);
      if (fileds.field[0] == 2 || fileds.field[0] == 3) {
        return true; // done processing
      }
      break;
    case '3': // field 3
      fileds.field[2] = FGetInteger(pch);
      if (fileds.field[0] == 1) {
        return true; // done processing
      }
      break;
    case '4': // field 4
      fileds.field[3] = FGetInteger(pch);
      return true; // done processing
    default:       // unknown field
      return false;
    }
    pch++; // for space (' ') between fields
  }
  return true;
}

bool Executor::empty() const {
  std::lock_guard<std::mutex> lock(mtx);
  return packets.empty();
}

void Executor::run() {

  //
}

bool Executor::InitializeExecutor() {
  t = std::make_shared<std::thread>([this]() {
    //
    run();
  });
  return false;
}

bool Executor::PushEvent(const std::wstring &msi, const std::wstring &outdir,
                         void *data) {
  canceled = false;
  (void)msi;
  (void)outdir;
  (void)data;
  return false;
}
} // namespace krycekium
