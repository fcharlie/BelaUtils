///
#include <bela/parseargv.hpp>
#include <bela/path.hpp>
#include <hazel/fs.hpp>
#include "bona.hpp"
#include "writer.hpp"
#include "resource.h"

namespace bona {
bool IsDebugMode = false;
bool IsFullMode = false;
bool AnalysisFile(std::wstring_view file, nlohmann::json *j) {
  bela::error_code ec;
  auto absPath = bela::PathAbsolute(file);
  auto realPath = absPath;
  hazel::fs::FileReparsePoint frp;
  bool areRsp = false;
  if (areRsp = hazel::fs::LookupReparsePoint(absPath, frp, ec); areRsp) {
    if (auto it = frp.attributes.find(L"Target"); it != frp.attributes.end()) {
      realPath = bela::PathAbsolute(it->second);
    }
  }
  bela::File fd;
  if (!fd.Open(realPath, ec)) {
    AppenError(j, file, ec);
    bela::FPrintF(stderr, L"Open file %s error: %s\n", file, ec.message);
    return false;
  }
  hazel::hazel_result hr;
  if (!hazel::LookupFile(fd, hr, ec)) {
    AppenError(j, file, ec);
    bela::FPrintF(stderr, L"Lookup file %s error: %s\n", file, ec.message);
    return false;
  }
  auto alen = hr.align_length();
  if (areRsp) {
    for (const auto &[k, v] : frp.attributes) {
      alen = (std::max)(alen, k.size());
    }
  }
  std::shared_ptr<Writer> w;
  if (j != nullptr) {
    w = std::make_shared<JsonWriter>(j);
  } else {
    w = std::make_shared<TextWriter>(alen);
  }
  w->Write(L"Description", hr.description());
  w->Write(L"Path", absPath);
  w->Write(L"Size", hr.size());
  if (auto it = hr.values().find(L"MIME"); it == hr.values().end()) {
    w->Write(L"MIME", hazel::LookupMIME(hr.type()));
  }
  if (areRsp) {
    for (const auto &[k, v] : frp.attributes) {
      w->Write(k, v);
    }
  }

  for (const auto &[k, v] : hr.values()) {
    w->WriteVariant(k, v);
  }
  if (hr.LooksLikePE()) {
    return AnalysisPE(fd, *w);
  }
  if (hr.LooksLikeELF()) {
    return AnalysisELF(fd, *w);
  }
  if (hr.LooksLikeMachO()) {
    return AnalysisMachO(fd, *w);
  }
  if (hr.LooksLikeZIP()) {
    return AnalysisZIP(fd, *w);
  }

  return true;
}

} // namespace bona

struct options {
  std::vector<std::wstring_view> files;
  int AnalysisResultToJson();
  int AnalysisResultToText();
  bool formatToJson{false};
  bool fullMode{false};
};

int options::AnalysisResultToJson() {
  nlohmann::json j;
  for (const auto file : files) {
    nlohmann::json cj;
    bona::AnalysisFile(file, &cj);
    j.emplace_back(std::move(cj));
  }
  try {
    bela::terminal::WriteAuto(stdout, j.dump(4));
    /* code */
  } catch (const std::exception &e) {
    bela::FPrintF(stderr, L"dump to json: %s\n", e.what());
    return 1;
  }
  return 0;
}
int options::AnalysisResultToText() {
  for (const auto file : files) {
    bona::AnalysisFile(file, nullptr);
    bela::FPrintF(stdout, L"\n");
  }
  return 0;
}

void Usage() {
  constexpr std::wstring_view usage = LR"(bona - Modern and interesting file format viewer
Usage: bona [option]... [file]...
  -h|--help        Show usage text and quit
  -v|--version     Show version number and quit
  -V|--verbose     Make the operation more talkative
  -F|--full        Full mode, view more detailed information of the file.
  -J|--json        Format and output file information into JSON.

)";
  bela::terminal::WriteAuto(stderr, usage);
}

void Version() {
  bela::FPrintF(
      stdout, L"bona %s - Modern and interesting file format viewer\nRelease:    %s\nCommit:     %s\nBuild Time: %s\n",
      BELAUTILS_VERSION, BELAUTILS_REFNAME, BELAUTILS_REVISION, BELAUTILS_BUILD_TIME);
}

bool ParseArgv(int argc, wchar_t **argv, options &opt) {
  bela::ParseArgv pa(argc, argv);
  pa.Add(L"help", bela::no_argument, 'h')
      .Add(L"version", bela::no_argument, 'v')
      .Add(L"verbose", bela::no_argument, 'V')
      .Add(L"json", bela::no_argument, 'J')
      .Add(L"full", bela::no_argument, 'F'); // Full mode
  bela::error_code ec;
  auto result = pa.Execute(
      [&](int val, const wchar_t *oa, const wchar_t *raw) {
        switch (val) {
        case 'h':
          Usage();
          exit(0);
        case 'v':
          Version();
          exit(0);
        case 'V':
          bona::IsDebugMode = true;
          break;
        case 'J':
          opt.formatToJson = true;
          break;
        case 'F':
          bona::IsFullMode = true;
          break;
        default:
          break;
        }
        return true;
      },
      ec);
  if (!result) {
    bela::FPrintF(stderr, L"ParseArgv: \x1b[31m%s\x1b[0m\n", ec.message);
    return false;
  }
  opt.files = pa.UnresolvedArgs();
  if (opt.files.empty()) {
    bela::FPrintF(stderr, L"bv: missing input file\n");
    return false;
  }
  return true;
}

int wmain(int argc, wchar_t **argv) {
  options opt;
  if (!ParseArgv(argc, argv, opt)) {
    return 1;
  }
  return opt.formatToJson ? opt.AnalysisResultToJson() : opt.AnalysisResultToText();
}