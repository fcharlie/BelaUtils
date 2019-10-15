#

add_library(hashlib STATIC
  hashlib.cc
)

target_include_directories(hashlib
  ../../vendor/blake2
  ../../vendor/rhash/librhash
)

target_link_libraries(hashlib
  rhash
  blake2
)
