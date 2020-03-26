#

add_library(hashlib STATIC hashlib.cc)

include_directories(hashlib ../../vendor/blake2 ../../vendor/rhash/librhash)

target_link_libraries(
  hashlib
  rhash
  blake2
  blake3
  bela)
