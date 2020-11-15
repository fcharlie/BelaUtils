#include <bela/base.hpp>
#include <bela/terminal.hpp>
#include <hazel.hpp>

bool WriteToJson(FILE *fd, const hazel::pe_particulars_result &pr);
// hazel check file details
int wmain(int argc, wchar_t **argv) {
  if (argc < 2) {
    bela::FPrintF(stderr, L"usage: %s file\n", argv[0]);
    return 1;
  }
  bela::error_code ec;
  auto pr = hazel::explore_file(argv[1], ec);
  if (!pr) {
    bela::FPrintF(stderr, L"unable look %s\n", ec.message);
    return 1;
  }
  if (pr->same_pecoff()) {
    auto per = hazel::explore_pecoff(argv[1], ec);
    if (per) {
      WriteToJson(stdout, *per);
    }
  }
  bela::FPrintF(stdout, L"description: %s mime: %s\n", pr->description(), pr->mime());
  return 0;
}