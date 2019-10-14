#

add_library(rhash STATIC
  librhash/aich.c
  librhash/byte_order.c
  librhash/ed2k.c
  librhash/edonr.c
  librhash/gost12.c
  librhash/gost94.c
  librhash/has160.c
  librhash/md4.c
  librhash/md5.c
  librhash/sha1.c
  librhash/sha256.c
  librhash/sha512.c
  librhash/sha3.c
)
