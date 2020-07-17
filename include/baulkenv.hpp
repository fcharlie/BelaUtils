///
#include <bela/base.hpp>
#include <bela/path.hpp>

namespace belautils {
inline bool PathFileIsExists(std::wstring_view file) {
  auto at = GetFileAttributesW(file.data());
  return (INVALID_FILE_ATTRIBUTES != at && (at & FILE_ATTRIBUTE_DIRECTORY) == 0);
}
class BaulkEnv {
public:
  BaulkEnv() = default;
  bool Initialize() {
    bela::error_code ec;
    auto p = bela::ExecutableParent(ec);
    if (!p) {
      return false;
    }
    // p --> baulk/bin/links
    // baulkroot (baulk/bin) --> (baulk)
    auto baulkroot_ = bela::DirName(bela::DirName(*p));
    if (baulkroot_.empty()) {
      return false;
    }
    auto baulkexe = bela::StringCat(baulkroot_, L"\\bin\\baulk.exe");
    if (!PathFileIsExists(baulkexe)) {
      return false;
    }
    baulkroot = baulkroot_;
    return true;
  }
  std::wstring_view BaulkRoot() const { return baulkroot; }

private:
  std::wstring baulkroot;
};
} // namespace belautils