#


add_executable(hazel
  driver.cc
  jsonwriter.cc
  hazel.rc
  hazel.manifest
)

target_link_libraries(hazel
  hazellib
  belawin
)

install(TARGETS hazel DESTINATION bin)