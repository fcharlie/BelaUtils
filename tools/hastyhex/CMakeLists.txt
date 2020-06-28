#


add_executable(hastyhex
  hastyhex.cc
)

target_link_libraries(hastyhex
  bela
)

install(TARGETS hastyhex DESTINATION bin)