///
#ifndef BONE_WRITER_HPP
#define BONE_WRITER_HPP
#include <variant>
#include <bela/datetime.hpp>
#include <bela/time.hpp>

namespace bona {
using bona_value_t = std::variant<std::string, std::wstring, std::vector<std::string>, std::vector<std::wstring>,
                                  int16_t, int32_t, int64_t, uint16_t, uint32_t, uint64_t, bela::Time>;
                                  class Writer{
                                      public:
                                  };
}

#endif