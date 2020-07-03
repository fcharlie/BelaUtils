///
#include <bela/hash.hpp>
#include <bela/match.hpp>
#include "sumizer.hpp"
#include "blake2.hpp"

namespace belautils {

inline void HashEncodeEx(const uint8_t *b, size_t len, std::wstring &hv, bool uppercase = false) {
  hv.resize(len * 2);
  auto p = hv.data();
  constexpr char hex[] = "0123456789abcdef";
  constexpr char uhex[] = "0123456789ABCDEF";
  if (uppercase) {
    for (size_t i = 0; i < len; i++) {
      uint32_t val = b[i];
      *p++ = uhex[val >> 4];
      *p++ = uhex[val & 0xf];
    }
    return;
  }
  for (size_t i = 0; i < len; i++) {
    uint32_t val = b[i];
    *p++ = hex[val >> 4];
    *p++ = hex[val & 0xf];
  }
}

class sha256sumizer : public Sumizer {
public:
  int Initialize(int w) {
    hasher.Initialize(w == 224 ? bela::hash::sha256::HashBits::SHA224
                               : bela::hash::sha256::HashBits::SHA256);
    return 0;
  }
  int Update(const unsigned char *b, size_t len) {
    hasher.Update(b, len);
    return 0;
  }
  int Final(std::wstring &hex, bool uc) {
    uint8_t buf[bela::hash::sha256::sha256_hash_size];
    hasher.Finalize(buf, static_cast<size_t>(hasher.hb) / 8);
    HashEncodeEx(buf, static_cast<size_t>(hasher.hb) / 8, hex, uc);
    return 0;
  }

private:
  bela::hash::sha256::Hasher hasher;
};

class sha512sumizer : public Sumizer {
public:
  int Initialize(int w) {
    hasher.Initialize(w == 384 ? bela::hash::sha512::HashBits::SHA384
                               : bela::hash::sha512::HashBits::SHA512);
    return 0;
  }
  int Update(const unsigned char *b, size_t len) {
    hasher.Update(b, len);
    return 0;
  }
  int Final(std::wstring &hex, bool uc) {
    uint8_t buf[bela::hash::sha512::sha512_hash_size];
    hasher.Finalize(buf, static_cast<size_t>(hasher.hb) / 8);
    HashEncodeEx(buf, static_cast<size_t>(hasher.hb) / 8, hex, uc);
    return 0;
  }

private:
  bela::hash::sha512::Hasher hasher;
};

class sha3sumizer : public Sumizer {
private:
  int Initialize(int w) {
    switch (w) {
    case 224:
      hasher.Initialize(bela::hash::sha3::HashBits::SHA3224);
      break;
    case 384:
      hasher.Initialize(bela::hash::sha3::HashBits::SHA3384);
      break;
    case 512:
      hasher.Initialize(bela::hash::sha3::HashBits::SHA3512);
      break;
    default:
      hasher.Initialize(bela::hash::sha3::HashBits::SHA3256);
      break;
    }
    return 0;
  }
  int Update(const unsigned char *b, size_t len) {
    hasher.Update(b, len);
    return 0;
  }
  int Final(std::wstring &hex, bool uc) {
    uint8_t buf[bela::hash::sha3::sha3_512_hash_size];
    hasher.Finalize(buf, static_cast<size_t>(hasher.hb) / 8);
    HashEncodeEx(buf, static_cast<size_t>(hasher.hb) / 8, hex, uc);
    return 0;
  }

private:
  bela::hash::sha3::Hasher hasher;
};

class blake2ssumizer : public Sumizer {
public:
  int Initialize(int w) {
    (void)w;
    return blake2s_init(&ctx, BLAKE2S_OUTBYTES);
  }
  int Update(const unsigned char *b, size_t len) { return blake2s_update(&ctx, b, len); }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[BLAKE2S_OUTBYTES];
    auto n = blake2s_final(&ctx, buf, BLAKE2S_OUTBYTES);
    if (n != 0) {
      return n;
    }
    HashEncodeEx(buf, BLAKE2S_OUTBYTES, hex, uc);
    return 0;
  }

private:
  blake2s_state ctx;
};

class blake2bsumizer : public Sumizer {
public:
  int Initialize(int w) {
    (void)w;
    return blake2b_init(&ctx, BLAKE2B_OUTBYTES);
  }
  int Update(const unsigned char *b, size_t len) {
    //
    return blake2b_update(&ctx, b, len);
  }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[BLAKE2B_OUTBYTES];
    auto n = blake2b_final(&ctx, buf, BLAKE2B_OUTBYTES);
    if (n != 0) {
      return n;
    }
    HashEncodeEx(buf, BLAKE2B_OUTBYTES, hex, uc);
    return 0;
  }

private:
  blake2b_state ctx;
};

class blake3sumizer : public Sumizer {
public:
  int Initialize(int w) {
    (void)w;
    hasher.Initialize();
    return 0;
  }
  int Update(const unsigned char *b, size_t len) {
    hasher.Update(b, len);
    return 0;
  }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[BLAKE3_OUT_LEN];
    hasher.Finalize(buf, BLAKE3_OUT_LEN);
    HashEncodeEx(buf, BLAKE3_OUT_LEN, hex, uc);
    return 0;
  }

private:
  bela::hash::blake3::Hasher hasher;
};

std::shared_ptr<Sumizer> make_sumizer(algorithm::hash_t alg) {
  using namespace algorithm;
  // Sumizer *sumizer = nullptr;
  std::shared_ptr<Sumizer> sumizer(nullptr);
  switch (alg) {
  case belautils::algorithm::hash_t::SHA224:
    sumizer = std::make_shared<sha256sumizer>();
    sumizer->Initialize(224);
    break;
  case belautils::algorithm::hash_t::SHA256:
    sumizer = std::make_shared<sha256sumizer>();
    sumizer->Initialize(256);
    break;
  case belautils::algorithm::hash_t::SHA384:
    sumizer = std::make_shared<sha512sumizer>();
    sumizer->Initialize(384);
    break;
  case belautils::algorithm::hash_t::SHA512:
    sumizer = std::make_shared<sha512sumizer>();
    sumizer->Initialize(512);
    break;
  case belautils::algorithm::hash_t::SHA3_224:
    sumizer = std::make_shared<sha3sumizer>();
    sumizer->Initialize(224);
    break;
  case belautils::algorithm::hash_t::SHA3_256:
    sumizer = std::make_shared<sha3sumizer>();
    sumizer->Initialize(256);
    break;
  case belautils::algorithm::hash_t::SHA3_384:
    sumizer = std::make_shared<sha3sumizer>();
    sumizer->Initialize(384);
    break;
  case belautils::algorithm::hash_t::BLAKE3:
    sumizer = std::make_shared<blake3sumizer>();
    sumizer->Initialize(256);
    break;
  case belautils::algorithm::hash_t::SHA3_512:
    sumizer = std::make_shared<sha3sumizer>();
    sumizer->Initialize(512);
    break;
  case belautils::algorithm::hash_t::BLAKE2S:
    sumizer = std::make_shared<blake2ssumizer>();
    sumizer->Initialize();
    break;
  case belautils::algorithm::hash_t::BLAKE2B:
    sumizer = std::make_shared<blake2bsumizer>();
    sumizer->Initialize();
    break;
  default:
    break;
  }
  return sumizer;
}

std::shared_ptr<Sumizer> make_sumizer(std::wstring_view alg) {
  using namespace algorithm;
  constexpr struct hash_algorithm_map {
    std::wstring_view s;
    hash_t h;
  } hav[] = {
      //
      {L"SHA224", belautils::algorithm::hash_t::SHA224},
      {L"SHA256", belautils::algorithm::hash_t::SHA256},
      {L"SHA384", belautils::algorithm::hash_t::SHA384},
      {L"SHA512", belautils::algorithm::hash_t::SHA512},
      {L"SHA3-224", belautils::algorithm::hash_t::SHA3_224},
      {L"SHA3-256", belautils::algorithm::hash_t::SHA3_256},
      {L"SHA3-384", belautils::algorithm::hash_t::SHA3_384},
      {L"SHA3-512", belautils::algorithm::hash_t::SHA3_512},
      {L"BLAKE3", belautils::algorithm::hash_t::BLAKE3},
      {L"BLAKE2s", belautils::algorithm::hash_t::BLAKE2S},
      {L"BLAKE2b", belautils::algorithm::hash_t::BLAKE2B}
      //
  };
  for (const auto &h : hav) {
    if (bela::EqualsIgnoreCase(h.s, alg)) {
      return make_sumizer(h.h);
    }
  }
  return nullptr;
}

algorithm::hash_t lookup_algorithm(std::wstring_view alg) {
  using namespace algorithm;
  constexpr struct hash_algorithm_map {
    std::wstring_view s;
    hash_t h;
  } hav[] = {
      //
      {L"MD5", belautils::algorithm::hash_t::MD5},
      {L"SHA1", belautils::algorithm::hash_t::SHA1},
      {L"SHA224", belautils::algorithm::hash_t::SHA224},
      {L"SHA256", belautils::algorithm::hash_t::SHA256},
      {L"SHA384", belautils::algorithm::hash_t::SHA384},
      {L"SHA512", belautils::algorithm::hash_t::SHA512},
      {L"SHA3-224", belautils::algorithm::hash_t::SHA3_224},
      {L"SHA3-256", belautils::algorithm::hash_t::SHA3_256},
      {L"SHA3-384", belautils::algorithm::hash_t::SHA3_384},
      {L"SHA3-512", belautils::algorithm::hash_t::SHA3_512},
      {L"BLAKE3", belautils::algorithm::hash_t::BLAKE3},
      {L"BLAKE2s", belautils::algorithm::hash_t::BLAKE2S},
      {L"BLAKE2b", belautils::algorithm::hash_t::BLAKE2B}
      //
  };
  for (const auto &h : hav) {
    if (bela::EqualsIgnoreCase(h.s, alg)) {
      return h.h;
    }
  }
  return NONE;
}

} // namespace belautils
