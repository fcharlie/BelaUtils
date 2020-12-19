// executor.cc
#include "krycekium.hpp"
#include "executor.hpp"
#include <bela/picker.hpp>
#include <windowsx.h>
#include <CommCtrl.h>
#include <Msi.h>

namespace krycekium {
// sender
// PostMessageW
class Sender {
public:
  Sender(HWND hWnd, HWND hProgress, const Executor &ex) : hWnd_(hWnd), hProgress_(hProgress), executor(ex) {
    //
  }
  Sender(const Sender &) = delete;
  Sender &operator=(const Sender &) = delete;
  void Progress(UINT msg, WPARAM wParam, LPARAM lParam) {
    // we don't known child control
    SendMessageW(hProgress_, msg, wParam, lParam);
  }
  void Notify(Status status, DWORD errcode) {
    //
    SendMessageW(hWnd_, WM_EXECUTOR_NOTIFY, (WPARAM)status, (LPARAM)errcode);
  }
  bool IsCanceled() const { return executor.IsCanceled(); }
  INT InstallHandler(UINT iMessageType, LPCWSTR szMessage);

private:
  void DoProgress(LPWSTR szMessage);
  HWND hWnd_;
  HWND hProgress_;
  const Executor &executor;
  bool firsttime{true};
  bool mEnableActionData{false};
  bool mForwardProgress{false};
  bool mScriptInProgress{false};
  int mProgressTotal{0};
  int mProgress{0};
  int iCurPos;
};

// install_ui_callback
INT WINAPI install_ui_callback(LPVOID ctx, UINT iMessageType, LPCWSTR szMessage) {
  auto sender = reinterpret_cast<Sender *>(ctx);
  return sender->InstallHandler(iMessageType, szMessage);
}
// https://docs.microsoft.com/zh-cn/windows/win32/msi/handling-progress-messages-using-msisetexternalui
struct ProgressFields {
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
bool ParseProgressString(LPWSTR sz, ProgressFields &Fields) {
  wchar_t *pch = sz;
  if (0 == *pch) {
    return false; // no msg
  }

  while (*pch != 0) {
    wchar_t chField = *pch++;
    pch++; // for ':'
    pch++; // for sp
    switch (chField) {
    case '1': // field 1
      // progress message type
      if (0 == isdigit(*pch)) {
        return false; // blank record
      }
      Fields.field[0] = *pch++ - '0';
      break;
    case '2': // field 2
      Fields.field[1] = FGetInteger(pch);
      if (Fields.field[0] == 2 || Fields.field[0] == 3) {
        return true; // done processing
      }
      break;
    case '3': // field 3
      Fields.field[2] = FGetInteger(pch);
      if (Fields.field[0] == 1) {
        return true; // done processing
      }
      break;
    case '4': // field 4
      Fields.field[3] = FGetInteger(pch);
      return true; // done processing
    default:       // unknown field
      return false;
    }
    pch++; // for space (' ') between fields
  }
  return true;
}

void Sender::DoProgress(LPWSTR szMessage) {
  ProgressFields pf;
  if (!ParseProgressString(szMessage, pf)) {
    return;
  }
  switch (pf.field[0]) {
  case 0:
    mProgressTotal = pf.field[1];
    mForwardProgress = (pf.field[2] == 0);
    mProgress = (mForwardProgress != 0 ? 0 : mProgressTotal);
    Progress(PBM_SETRANGE32, 0, mProgressTotal);
    Progress(PBM_SETPOS, mScriptInProgress ? mProgressTotal : mProgress, 0);
    iCurPos = 0;
    mScriptInProgress = (pf.field[3] == 1);
    break;
  case 1:
    if (pf.field[2] != 0) {
      Progress(PBM_SETSTEP, mForwardProgress ? pf.field[1] : -1 * pf.field[1], 0);
      mEnableActionData = true;
      break;
    }
    mEnableActionData = false;
    break;
  case 2:
    if (mProgressTotal != 0) {
      iCurPos += pf.field[1];
      Progress(PBM_SETPOS, mForwardProgress ? iCurPos : -1 * iCurPos, 0);
    }
    break;
  default:
    break;
  }
}

INT Sender::InstallHandler(UINT iMessageType, LPCWSTR szMessage) {
  if (firsttime) {
    MsiSetInternalUI(INSTALLUILEVEL_BASIC, nullptr);
    firsttime = false;
  }
  if (IsCanceled()) {
    return IDCANCEL;
  }
  if (szMessage == nullptr) {
    return 0;
  }
  auto mt = static_cast<INSTALLMESSAGE>(0xFF000000 & iMessageType);
  auto uiflags = 0x00FFFFFF & iMessageType;
  switch (mt) {
  case INSTALLMESSAGE_FATALEXIT:
    return 1;
  case INSTALLMESSAGE_ERROR: {
    MessageBeep(uiflags & MB_ICONMASK);
    return ::MessageBoxEx(hWnd_, szMessage, TEXT("Error"), uiflags, LANG_NEUTRAL);
  };
  case INSTALLMESSAGE_WARNING:
    bela::BelaMessageBox(hWnd_, L"Install message warning", szMessage, nullptr, bela::mbs_t::WARN);
    break;
  case INSTALLMESSAGE_USER:
    return IDOK;
  case INSTALLMESSAGE_INFO:
    return IDOK;
  case INSTALLMESSAGE_FILESINUSE:
    /* Display FilesInUse dialog */
    // parse the message text to provide the names of the
    // applications that the user can close so that the
    // files are no longer in use.
    return 0;

  case INSTALLMESSAGE_RESOLVESOURCE:
    /* ALWAYS return 0 for ResolveSource */
    return 0;

  case INSTALLMESSAGE_OUTOFDISKSPACE:
    /* Get user message here */
    return IDOK;

  case INSTALLMESSAGE_ACTIONSTART:
    /* New action started, any action data is sent by this new action */
    mEnableActionData = false;
    return IDOK;
  case INSTALLMESSAGE_PROGRESS:
    DoProgress(const_cast<LPWSTR>(szMessage));
    return IDOK;
  case INSTALLMESSAGE_INITIALIZE:
    return IDOK;
  // Sent after UI termination, no string data
  case INSTALLMESSAGE_TERMINATE:
    return IDOK;
  // Sent prior to display of authored dialog or wizard
  case INSTALLMESSAGE_SHOWDIALOG:
    return IDOK;
  default:
    break;
  }
  return IDOK;
}

bool Executor::execute(Packet &pk) {
  auto cmd = bela::StringCat(L"ACTION=ADMIN TARGETDIR=\"", pk.outdir, L"\"");
  Sender sender(pk.hWnd, pk.hProgress, *this);
  MsiSetInternalUI(INSTALLUILEVEL(INSTALLUILEVEL_NONE | INSTALLUILEVEL_SOURCERESONLY), nullptr);
  MsiSetExternalUIW(install_ui_callback,
                    INSTALLLOGMODE_PROGRESS | INSTALLLOGMODE_FATALEXIT | INSTALLLOGMODE_ERROR | INSTALLLOGMODE_WARNING |
                        INSTALLLOGMODE_USER | INSTALLLOGMODE_INFO | INSTALLLOGMODE_RESOLVESOURCE |
                        INSTALLLOGMODE_OUTOFDISKSPACE | INSTALLLOGMODE_ACTIONSTART | INSTALLLOGMODE_ACTIONDATA |
                        INSTALLLOGMODE_COMMONDATA | INSTALLLOGMODE_PROGRESS | INSTALLLOGMODE_INITIALIZE |
                        INSTALLLOGMODE_TERMINATE | INSTALLLOGMODE_SHOWDIALOG,
                    &sender);
  if (MsiInstallProductW(pk.msi.data(), cmd.data()) == ERROR_SUCCESS) {
    sender.Notify(krycekium::Status::Completed, 0);
    return true;
  }
  sender.Notify(krycekium::Status::Failure, GetLastError());
  return true;
}

void Executor::run() {
  for (;;) {
    Packet pack;
    {
      std::unique_lock<std::recursive_mutex> lock(mtx);
      cv.wait(lock, [this]() {
        //
        return this->exited || !this->packets.empty();
      });
      if (this->exited || this->packets.empty()) {
        return;
      }
      pack = std::move(packets.front());
      packets.pop();
    }
    execute(pack);
    // execute one task
  }
}

bool Executor::InitializeExecutor() {
  t = std::make_shared<std::thread>([this]() {
    // run some thing
    run();
  });
  return !!t;
}

bool Executor::PushEvent(std::wstring_view msi, std::wstring_view outdir, HWND hWnd, HWND hProgress) {
  canceled = false;
  {
    std::unique_lock<std::recursive_mutex> lock(mtx);
    packets.emplace(msi, outdir, hWnd, hProgress);
  }
  cv.notify_one();
  return true;
}
} // namespace krycekium
