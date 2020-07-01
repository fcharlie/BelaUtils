#


add_executable(hastyhex
  hastyhex.cc
  hastyhex.rc
  hastyhex.manifest
)

target_link_libraries(hastyhex
  bela
)

install(TARGETS hastyhex DESTINATION bin)