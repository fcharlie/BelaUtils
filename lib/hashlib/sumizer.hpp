///
#ifndef BELAUTILS_HASHLIB_SUMIZER_HPP
#define BELAUTILS_HASHLIB_SUMIZER_HPP
#include <string_view>
#include <memory>

namespace belautils {
class Sumizer {
public:
  virtual int Initialize(int w = 0) = 0;
  virtual int Update(const unsigned char *b, size_t len) = 0;
  virtual int Final(std::wstring &hex, bool uc = false) = 0;

private:
};

// cast to ASCII, such as hex code and string ....
template <typename T> std::string string_cast(T value) {
  std::string s;
  s.resize(value.size());
  auto p = s.data();
  for (auto c : value) {
    *p++ = static_cast<char>(c);
  }
  return s;
}

namespace algorithm {
enum class hash_t : int {
  MD5,
  SHA1,
  SHA224,
  SHA256,
  SHA384,
  SHA512,
  SHA3_224,
  SHA3_256,
  SHA3_384,
  SHA3_512,
  BLAKE2S,
  BLAKE2B,
  BLAKE3,
  KangarooTwelve,
  SM3,
  NONE = 999
};
[[maybe_unused]] constexpr auto NONE = hash_t::NONE;
} // namespace algorithm

// md5 sha1 sha224 sha256 sha384 sha512
// sha3-224 sha3-256 sha3-384 sha3-512
// blake2s blake2b KangarooTwelve
algorithm::hash_t lookup_algorithm(std::wstring_view alg);
std::shared_ptr<Sumizer> make_sumizer(algorithm::hash_t alg);
std::shared_ptr<Sumizer> make_sumizer(std::wstring_view alg);
} // namespace belautils

#endif
