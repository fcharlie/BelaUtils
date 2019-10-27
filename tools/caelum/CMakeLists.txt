#

add_executable(caelum WIN32
  caelum.cc
  pecoff.cc
  window.cc
  caelum.rc
  caelum.manifest
)
# no atl depend
target_link_libraries(caelum
  belawin
  belashl
  d2d1
  dwrite
  Shell32
)