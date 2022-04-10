///
#ifndef BONE_WRITER_HPP
#define BONE_WRITER_HPP
#include <bela/datetime.hpp>
#include <bela/time.hpp>
#include <bela/ascii.hpp>
#include <bela/codecvt.hpp>
#include <bela/terminal.hpp>
#include <bela/str_cat.hpp>
#include <bela/pe.hpp>
#include <bela/und.hpp>
#include <hazel/hazel.hpp>
#include <json.hpp>

namespace bona {
namespace internal {
struct intName {
  int i;
  const char *val;
};

inline std::string stringNameInternal(uint32_t val, const intName *in, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (in[i].i == val) {
      return in[i].val;
    }
  }
  return std::string(bela::AlphaNumNarrow(val).Piece());
}

template <typename T, size_t N> std::string stringName(T v, const intName (&in)[N]) {
  return stringNameInternal(static_cast<uint32_t>(v), in, N);
}
} // namespace internal

using bona_value_t = hazel::hazel_value_t;
class Writer {
public:
  virtual void WriteVariant(std::wstring_view key, const bona_value_t &val) = 0;
  virtual void WriteError(const bela::error_code &ec) = 0;
  virtual void WriteAddress(std::wstring_view k, std::ptrdiff_t val) = 0;
  virtual void WriteBool(std::wstring_view k, bool val) = 0;
  virtual void Write(std::wstring_view k, std::int64_t val) = 0;
  virtual void Write(std::wstring_view k, std::uint64_t val) = 0;
  virtual void Write(std::wstring_view k, std::int32_t val) = 0;
  virtual void Write(std::wstring_view k, std::uint32_t val) = 0;
  virtual void Write(std::wstring_view k, std::int16_t val) = 0;
  virtual void Write(std::wstring_view k, std::uint16_t val) = 0;
  virtual void Write(std::wstring_view k, bela::Time val) = 0;
  virtual void Write(std::wstring_view k, std::string_view val) = 0;
  virtual void Write(std::wstring_view k, std::wstring_view val) = 0;
  virtual void Write(std::wstring_view k, const std::vector<std::string> &val) = 0;
  virtual void Write(std::wstring_view k, const std::vector<std::wstring> &val) = 0;
  virtual void Write(std::wstring_view name) = 0;
  virtual void Write(std::wstring_view k, std::string_view d, const std::vector<bela::pe::Function> &funs,
                     bela::pe::SymbolSearcher &sse) = 0;
  virtual nlohmann::json *Raw() = 0;
};

inline void to_json(nlohmann::json &j, const bela::pe::Function &func) {
  j = nlohmann::json{{"name", func.Name}, {"index", func.Index}, {"ordinal", func.Ordinal}};
}

inline std::string demangle(const std::string &name) {
  if (name.empty()) {
    return name;
  }
  return bela::demangle(name);
}

class JsonWriter : public Writer {
public:
  JsonWriter(nlohmann::json *j_) : j(j_) {}
  void WriteVariant(std::wstring_view key, const bona_value_t &val) {
    auto nk = bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(key));
    std::visit(hazel::overloaded{
                   [](auto arg) {}, // ignore
                   [&](const std::wstring &sv) { j->emplace(nk, bela::encode_into<wchar_t, char>(sv)); },
                   [&](const std::string &sv) { j->emplace(nk, sv); },
                   [&](int16_t i) { j->emplace(nk, i); },
                   [&](int32_t i) { j->emplace(nk, i); },
                   [&](int64_t i) { j->emplace(nk, i); },
                   [&](uint16_t i) { j->emplace(nk, i); },
                   [&](uint32_t i) { j->emplace(nk, i); },
                   [&](uint64_t i) { j->emplace(nk, i); },
                   [&](bela::Time t) { j->emplace(nk, bela::encode_into<wchar_t, char>(bela::FormatTime(t))); },
                   [&](const std::vector<std::wstring> &v) {
                     auto av = nlohmann::json::array();
                     for (const auto s : v) {
                       av.emplace_back(bela::encode_into<wchar_t, char>(s));
                     }
                     j->emplace(nk, std::move(av));
                   },
                   [&](const std::vector<std::string> &v) { j->emplace(nk, v); },
               },
               val);
  }
  void WriteError(const bela::error_code &ec) {
    j->emplace("code", ec.code);
    j->emplace("message", bela::encode_into<wchar_t, char>(ec.message));
  }
  void WriteAddress(std::wstring_view k, std::ptrdiff_t val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }
  void WriteBool(std::wstring_view k, bool val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }
  void Write(std::wstring_view k, std::int64_t val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }
  void Write(std::wstring_view k, std::uint64_t val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }
  void Write(std::wstring_view k, std::int32_t val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }
  void Write(std::wstring_view k, std::uint32_t val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }
  void Write(std::wstring_view k, std::int16_t val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }
  void Write(std::wstring_view k, std::uint16_t val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }

  void Write(std::wstring_view k, bela::Time val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), bela::FormatUniversalTime<char>(val));
  }
  void Write(std::wstring_view k, std::string_view val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }
  void Write(std::wstring_view k, std::wstring_view val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), bela::encode_into<wchar_t, char>(val));
  }
  void Write(std::wstring_view k, const std::vector<std::string> &val) {
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), val);
  }
  void Write(std::wstring_view k, const std::vector<std::wstring> &val) {
    std::vector<std::string> av;
    for (const auto &v : val) {
      av.emplace_back(bela::encode_into<wchar_t, char>(v));
    }
    j->emplace(bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k)), std::move(av));
  }
  void Write(std::wstring_view /*name*/) {}
  void Write(std::wstring_view k, std::string_view d, const std::vector<bela::pe::Function> &funs,
             bela::pe::SymbolSearcher &sse) {
    try {
      bela::error_code ec;
      auto o = nlohmann::json::array();
      for (const auto &n : funs) {
        if (n.Ordinal != 0) {
          if (auto fn = sse.LookupOrdinalFunctionName(d, n.Ordinal, ec); fn) {
            o.push_back(nlohmann::json{{"name", demangle(*fn)}, {"index", n.Index}, {"ordinal", n.Ordinal}});
            continue;
          }
        }
        o.push_back(nlohmann::json{{"name", demangle(n.Name)}, {"index", n.Index}, {"ordinal", n.Ordinal}});
      }
      auto lk = bela::encode_into<wchar_t, char>(bela::AsciiStrToLower(k));
      if (auto it = j->find(lk); it != j->end()) {
        it.value().emplace(d, std::move(o));
        return;
      }
      nlohmann::json jo;
      jo.emplace(d, std::move(o));
      j->emplace(lk, std::move(jo));
    } catch (const std::exception &e) {
      bela::FPrintF(stderr, L"Write Functions: %s\n", e.what());
    }
  }
  nlohmann::json *Raw() { return j; }

private:
  nlohmann::json *j{nullptr};
};
constexpr size_t amlen = 12;
class TextWriter : public Writer {
public:
  TextWriter(size_t al) : alen((std::max)(al, amlen)) { space.resize(alen + 2, ' '); }
  void WriteVariant(std::wstring_view key, const bona_value_t &val) {
    std::wstring_view spaceview{space};
    bela::FPrintF(stderr, L"%v:%s", key, spaceview.substr(0, spaceview.size() - key.size() - 1));
    std::visit(hazel::overloaded{
                   [](auto arg) {}, // ignore
                   [](const std::wstring &sv) { bela::FPrintF(stdout, L"%s\n", sv); },
                   [](const std::string &sv) { bela::FPrintF(stdout, L"%s\n", sv); },
                   [](int16_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](int32_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](int64_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](uint16_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](uint32_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](uint64_t i) { bela::FPrintF(stdout, L"%d\n", i); },
                   [](bela::Time t) { bela::FPrintF(stdout, L"%s\n", bela::FormatTime(t)); },
                   [spaceview](const std::vector<std::wstring> &v) {
                     if (v.empty()) {
                       bela::FPrintF(stdout, L"\n");
                       return;
                     }
                     bela::FPrintF(stdout, L"%s\n", v[0]);
                     for (size_t i = 1; i < v.size(); i++) {
                       bela::FPrintF(stdout, L"%s%s\n", spaceview, v[0]);
                     }
                   },
                   [spaceview](const std::vector<std::string> &v) {
                     if (v.empty()) {
                       bela::FPrintF(stdout, L"\n");
                       return;
                     }
                     bela::FPrintF(stdout, L"%s\n", v[0]);
                     for (size_t i = 1; i < v.size(); i++) {
                       bela::FPrintF(stdout, L"%s%s\n", spaceview, v[0]);
                     }
                   },
               },
               val);
  }
  void WriteError(const bela::error_code &ec) {
    std::wstring_view spaceview{space};
    bela::FPrintF(stdout, L"ErrorCode:%s%d\n", spaceview.substr(0, spaceview.size() - 9 - 1), ec.code);
    bela::FPrintF(stdout, L"Message:%s%s\n", spaceview.substr(0, spaceview.size() - 7 - 1), ec.message);
  }
  void WriteAddress(std::wstring_view k, std::ptrdiff_t val) {
    std::wstring_view spaceview{space};
    bela::AlphaNum an(bela::Hex(val, bela::kZeroPad8));
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s0x%s\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), an.Piece());
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s0x%s\n", k, spaceview, an.Piece());
  }
  void WriteBool(std::wstring_view k, bool val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%b\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), val);
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%b\n", k, spaceview, val);
  }
  void Write(std::wstring_view k, std::int64_t val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%d\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), val);
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%d\n", k, spaceview, val);
  }
  void Write(std::wstring_view k, std::uint64_t val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%d\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), val);
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%d\n", k, spaceview, val);
  }
  void Write(std::wstring_view k, std::int32_t val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%d\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), val);
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%d\n", k, spaceview, val);
  }
  void Write(std::wstring_view k, std::uint32_t val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%d\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), val);
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%d\n", k, spaceview, val);
  }
  void Write(std::wstring_view k, std::int16_t val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%d\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), val);
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%d\n", k, spaceview, val);
  }
  void Write(std::wstring_view k, std::uint16_t val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%d\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), val);
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%d\n", k, spaceview, val);
  }
  void Write(std::wstring_view k, bela::Time val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%s\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1),
                    bela::FormatTime(val));
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%s\n", k, spaceview, bela::FormatTime(val));
  }
  void Write(std::wstring_view k, std::string_view val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%s\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), val);
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%s\n", k, spaceview, val);
  }
  void Write(std::wstring_view k, std::wstring_view val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s%s\n", k, spaceview.substr(0, spaceview.size() - k.size() - 1), val);
      return;
    }
    bela::FPrintF(stdout, L"%s:\n%s%s\n", k, spaceview, val);
  }
  void Write(std::wstring_view k, const std::vector<std::string> &val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s", k, spaceview.substr(0, spaceview.size() - k.size() - 1));
    } else {
      bela::FPrintF(stdout, L"%s:\n%s", k, spaceview);
    }
    if (val.empty()) {
      bela::FPrintF(stdout, L"[]\n");
      return;
    }
    bela::FPrintF(stdout, L"%s\n", val[0]);
    for (size_t i = 1; i < val.size(); i++) {
      bela::FPrintF(stdout, L"%s%s\n", spaceview, val[i]);
    }
  }
  void Write(std::wstring_view k, const std::vector<std::wstring> &val) {
    std::wstring_view spaceview{space};
    if (spaceview.size() >= k.size() + 2) {
      bela::FPrintF(stdout, L"%s:%s", k, spaceview.substr(0, spaceview.size() - k.size() - 1));
    } else {
      bela::FPrintF(stdout, L"%s:\n%s", k, spaceview);
    }
    if (val.empty()) {
      bela::FPrintF(stdout, L"[]\n");
      return;
    }
    bela::FPrintF(stdout, L"%s\n", val[0]);
    for (size_t i = 1; i < val.size(); i++) {
      bela::FPrintF(stdout, L"%s%s\n", spaceview, val[i]);
    }
  }
  void Write(std::wstring_view name) { bela::FPrintF(stdout, L"%s:\n", name); }
  void Write(std::wstring_view k, std::string_view d, const std::vector<bela::pe::Function> &funs,
             bela::pe::SymbolSearcher &sse) {
    std::wstring_view spaceview{space};
    bela::FPrintF(stdout, L"\x1b[34m%s:\x1b[0m\n", d);
    bela::error_code ec;
    for (const auto &n : funs) {
      bela::AlphaNum an(n.Index);
      auto sv = spaceview.substr(0, 10 - an.Piece().size());
      if (n.Ordinal == 0) {
        bela::FPrintF(stdout, L"%s%s %s\n", sv, an.Piece(), bela::demangle(n.Name));
        continue;
      }
      if (auto fn = sse.LookupOrdinalFunctionName(d, n.Ordinal, ec); fn) {
        bela::FPrintF(stdout, L"%s%s %s (Ordinal %d)\n", sv, an.Piece(), bela::demangle(*fn), n.Ordinal);
        continue;
      }
      bela::FPrintF(stdout, L"%s%s Ordinal%d (Ordinal %d)\n", sv, an.Piece(), n.Ordinal, n.Ordinal);
    }
  }
  nlohmann::json *Raw() { return nullptr; }

private:
  std::wstring space;
  size_t alen{0};
};

bool AnalysisPE(bela::io::FD &fd, Writer &w);
bool AnalysisELF(bela::io::FD &fd, Writer &w);
bool AnalysisMachO(bela::io::FD &fd, Writer &w);
bool AnalysisZIP(bela::io::FD &fd, Writer &w, int64_t offset = 0);

} // namespace bona

#endif