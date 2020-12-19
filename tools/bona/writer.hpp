///
#ifndef BONE_WRITER_HPP
#define BONE_WRITER_HPP
#include <variant>
#include <bela/datetime.hpp>
#include <bela/time.hpp>
#include <json.hpp>

namespace bona {
using bona_value_t = std::variant<std::string, std::wstring, std::vector<std::string>, std::vector<std::wstring>,
                                  int16_t, int32_t, int64_t, uint16_t, uint32_t, uint64_t, bela::Time>;
class Writer {
public:
  virtual bool Write(std::wstring_view key, const bona_value_t &val) = 0;
};

class JsonWriter : public Writer {
public:
  JsonWriter(nlohmann::json *j_) : j(j_) {}
  bool Write(std::wstring_view key, const bona_value_t &val) {
    //
    return true;
  }

private:
  nlohmann::json *j{nullptr};
};

class TextWriter : public Writer {
public:
  TextWriter(size_t al) : alen(al) {}
  bool Write(std::wstring_view key, const bona_value_t &val) {
    //
    return true;
  }

private:
  std::wstring space;
  size_t alen{0};
};

} // namespace bona

#endif