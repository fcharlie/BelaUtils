///
#include "../rhash/librhash/sha256.c"
#include "../rhash/librhash/sha512.c"
#include "../rhash/librhash/byte_order.c"

unsigned char *SHA256(const unsigned char *d, size_t n, unsigned char *md) {
  sha256_ctx ctx;
  rhash_sha256_init(&ctx);
  rhash_sha256_update(&ctx, d, n);
  rhash_sha256_final(&ctx, md);
  memset(&ctx, 0, sizeof(ctx));
  return md;
}
unsigned char *SHA512(const unsigned char *d, size_t n, unsigned char *md) {
  sha512_ctx ctx;
  rhash_sha512_init(&ctx);
  rhash_sha512_update(&ctx, d, n);
  rhash_sha512_final(&ctx, md);
  memset(&ctx, 0, sizeof(ctx));
  return md;
}
