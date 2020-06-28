# kisasum

add_executable(kisasum-ui WIN32
  ui/kisasum.cc
  ui/kisasumutils.cc
  ui/ui.cc
  ui/kisasum.rc
  ui/kisasum.manifest
)

target_link_libraries(kisasum-ui
  hashlib
  belawin
  belashl
  d2d1
  dwrite
  Shell32
)

add_executable(kisasum
  cli/kisasum.cc
  cli/indicators.cc
)

target_link_libraries(kisasum
  hashlib
  belawin
)

install(TARGETS kisasum-ui DESTINATION bin)
install(TARGETS kisasum DESTINATION bin)