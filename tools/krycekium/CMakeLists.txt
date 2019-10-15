#

add_executable(krycekium WIN32
  krycekium.cc
  executor.cc
  window.cc
)

target_link_libraries(krycekium
  belawin
)