#

add_executable(krycekium WIN32
  krycekium.cc
  executor.cc
  window.cc
  krycekium.rc
)

target_link_libraries(krycekium
  belawin
)