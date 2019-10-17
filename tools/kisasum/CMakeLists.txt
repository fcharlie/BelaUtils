# kisasum

add_executable(kisasum-ui WIN32
  kisasum-ui.cc
)

target_link_libraries(kisasum-ui
  hashlib
)

add_executable(kisasum
  kisasum.cc
)

target_link_libraries(kisasum
  hashlib
)
