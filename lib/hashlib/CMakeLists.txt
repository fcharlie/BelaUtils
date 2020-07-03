#

add_library(hashlib STATIC hashlib.cc)

target_link_libraries(hashlib blake2 belahash)
