///
#include "../../vendor/rhash/librhash/md5.h"
#include "../../vendor/rhash/librhash/sha1.h"
#include "../../vendor/rhash/librhash/sha256.h"
#include "../../vendor/rhash/librhash/sha3.h"
#include "../../vendor/rhash/librhash/sha512.h"
#include "../../vendor/blake2/blake2.h"
#include <bela/match.hpp>
#include "sumizer.hpp"

namespace belautils {
// hashlib feature
inline void binary_to_hex(const unsigned char *b, size_t len, std::wstring &hex,
                          bool uppercase = false) {
  hex.resize(len * 2);
  auto p = hex.data();
  constexpr char lower_hex[] = "0123456789abcdef";
  constexpr char upper_hex[] = "0123456789ABCDEF";
  if (uppercase) {
    for (size_t i = 0; i < len; i++) {
      unsigned int val = b[i];
      *p++ = upper_hex[val >> 4];
      *p++ = upper_hex[val & 0xf];
    }
  } else {
    for (size_t i = 0; i < len; i++) {
      unsigned int val = b[i];
      *p++ = lower_hex[val >> 4];
      *p++ = lower_hex[val & 0xf];
    }
  }
}

class md5sumizer : public Sumizer {
public:
  int Initialize(int w) {
    (void)w;
    rhash_md5_init(&ctx);
    return 0;
  }
  int Update(const unsigned char *b, size_t len) {
    rhash_md5_update(&ctx, b, len);
    return 0;
  }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[md5_hash_size + 1];
    rhash_md5_final(&ctx, buf);
    binary_to_hex(buf, md5_hash_size, hex, uc);
    return 0;
  }

private:
  md5_ctx ctx;
};

class sha1sumizer : public Sumizer {
public:
  int Initialize(int w) {
    (void)w;
    rhash_sha1_init(&ctx);
    return 0;
  }
  int Update(const unsigned char *b, size_t len) {
    rhash_sha1_update(&ctx, b, len);
    return 0;
  }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[sha1_hash_size + 1];
    rhash_sha1_final(&ctx, buf);
    binary_to_hex(buf, sha1_hash_size, hex, uc);
    return 0;
  }

private:
  sha1_ctx ctx;
};

class sha256sumizer : public Sumizer {
public:
  int Initialize(int w) {
    if (w == 224) {
      width = w;
      rhash_sha224_init(&ctx);
    } else {
      rhash_sha256_init(&ctx);
    }
    return 0;
  }
  int Update(const unsigned char *b, size_t len) {
    rhash_sha256_update(&ctx, b, len);
    return 0;
  }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[sha256_hash_size + 1];
    rhash_sha256_final(&ctx, buf);
    binary_to_hex(buf, width / 8, hex, uc);
    return 0;
  }

private:
  sha256_ctx ctx;
  int width{256};
};

class sha512sumizer : public Sumizer {
public:
  int Initialize(int w) {
    if (w == 384) {
      width = 384;
      rhash_sha384_init(&ctx);
    } else {
      rhash_sha512_init(&ctx);
    }
    return 0;
  }
  int Update(const unsigned char *b, size_t len) {
    rhash_sha512_update(&ctx, b, len);
    return 0;
  }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[sha512_hash_size + 1];
    rhash_sha512_final(&ctx, buf);
    binary_to_hex(buf, width / 8, hex, uc);
    return 0;
  }

private:
  sha512_ctx ctx;
  int width{512};
};

class sha3sumizer : public Sumizer {
private:
  int Initialize(int w) {
    switch (w) {
    case 224:
      width = 224;
      rhash_sha3_224_init(&ctx);
      break;
    case 384:
      width = 384;
      rhash_sha3_384_init(&ctx);
      break;
    case 512:
      width = 512;
      rhash_sha3_512_init(&ctx);
      break;
    default:
      rhash_sha3_256_init(&ctx);
      break;
    }
    return 0;
  }
  int Update(const unsigned char *b, size_t len) {
    rhash_sha3_update(&ctx, b, len);
    return 0;
  }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[sha3_512_hash_size + 1];
    rhash_sha3_final(&ctx, buf);
    binary_to_hex(buf, width / 8, hex, uc);
    return 0;
  }

private:
  sha3_ctx ctx;
  int width{256};
};

class blake2ssumizer : public Sumizer {
public:
  int Initialize(int w) {
    (void)w;
    return blake2s_init(&ctx, BLAKE2S_OUTBYTES);
  }
  int Update(const unsigned char *b, size_t len) {
    return blake2s_update(&ctx, b, len);
  }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[BLAKE2S_OUTBYTES];
    auto n = blake2s_final(&ctx, buf, BLAKE2S_OUTBYTES);
    if (n != 0) {
      return n;
    }
    binary_to_hex(buf, BLAKE2S_OUTBYTES, hex, uc);
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
    return blake2b_update(&ctx, b, len);
  }
  int Final(std::wstring &hex, bool uc) {
    unsigned char buf[BLAKE2B_OUTBYTES];
    auto n = blake2b_final(&ctx, buf, BLAKE2B_OUTBYTES);
    if (n != 0) {
      return n;
    }
    binary_to_hex(buf, BLAKE2B_OUTBYTES, hex, uc);
    return 0;
  }

private:
  blake2b_state ctx;
};

std::shared_ptr<Sumizer> make_sumizer(algorithm::hash_t alg) {
  using namespace algorithm;
  // Sumizer *sumizer = nullptr;
  std::shared_ptr<Sumizer> sumizer(nullptr);
  switch (alg) {
  case MD5:
    sumizer = std::make_shared<md5sumizer>();
    sumizer->Initialize();
    break;
  case SHA1:
    sumizer = std::make_shared<sha1sumizer>();
    sumizer->Initialize();
    break;
  case SHA224:
    sumizer = std::make_shared<sha256sumizer>();
    sumizer->Initialize(224);
    break;
  case SHA256:
    sumizer = std::make_shared<sha256sumizer>();
    sumizer->Initialize(256);
    break;
  case SHA384:
    sumizer = std::make_shared<sha512sumizer>();
    sumizer->Initialize(384);
    break;
  case SHA512:
    sumizer = std::make_shared<sha512sumizer>();
    sumizer->Initialize(512);
    break;
  case SHA3_224:
    sumizer = std::make_shared<sha3sumizer>();
    sumizer->Initialize(224);
    break;
  case SHA3_256:
    sumizer = std::make_shared<sha3sumizer>();
    sumizer->Initialize(256);
    break;
  case SHA3_384:
    sumizer = std::make_shared<sha3sumizer>();
    sumizer->Initialize(384);
    break;
  case SHA3_512:
    sumizer = std::make_shared<sha3sumizer>();
    sumizer->Initialize(512);
    break;
  case BLAKE2S:
    sumizer = std::make_shared<blake2ssumizer>();
    sumizer->Initialize();
    break;
  case BLAKE2B:
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
      {L"MD5", MD5},
      {L"SHA1", SHA1},
      {L"SHA224", SHA224},
      {L"SHA256", SHA256},
      {L"SHA384", SHA384},
      {L"SHA512", SHA512},
      {L"SHA3-224", SHA3_224},
      {L"SHA3-256", SHA3_256},
      {L"SHA3-384", SHA3_384},
      {L"SHA3-512", SHA3_512},
      {L"BLAKE2s", BLAKE2S},
      {L"BLAKE2b", BLAKE2B}
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
      {L"MD5", MD5},
      {L"SHA1", SHA1},
      {L"SHA224", SHA224},
      {L"SHA256", SHA256},
      {L"SHA384", SHA384},
      {L"SHA512", SHA512},
      {L"SHA3-224", SHA3_224},
      {L"SHA3-256", SHA3_256},
      {L"SHA3-384", SHA3_384},
      {L"SHA3-512", SHA3_512},
      {L"BLAKE2s", BLAKE2S},
      {L"BLAKE2b", BLAKE2B}
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
