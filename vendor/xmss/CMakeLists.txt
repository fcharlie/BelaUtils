# https://github.com/XMSS/xmss-reference/tree/fb7e3f8edce8d412a707f522d597ab3546863202
add_library(XMSS STATIC
  fips202.c
  hash.c
  params.c
  randombytes.c
  sha.c
  utils.c
  wots.c
  xmss_commons.cc
  xmss_core_fast.c # we use xmss_core_fast.c not xmss_core.c
  xmss.c
)
