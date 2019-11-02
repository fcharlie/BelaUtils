# kisasum

add_executable(kisasum-ui WIN32
  ui/kisasum-ui.cc
  ui/kisasumutils.cc
  ui/ui.cc
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
)

target_link_libraries(kisasum
  hashlib
  belawin
)
