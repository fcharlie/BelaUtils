# kisasum

# kisasumlib

add_library(kisasumlib STATIC
  kisasumlib.cc
)


add_executable(kisasum-ui WIN32
  kisasum-ui.cc
)

target_link_libraries(kisasum-ui
  kisasumlib
)

add_executable(kisasum
  kisasum.cc
)

target_link_libraries(kisasum
  kisasumlib
)
