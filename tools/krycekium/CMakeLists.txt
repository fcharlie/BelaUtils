#

add_executable(
  krycekium WIN32
  krycekium.cc
  executor.cc
  window.cc
  kmutils.cc
  krycekium.rc
  krycekium.manifest)

target_link_libraries(
  krycekium
  belawin
  belashl
  d2d1
  dwrite
  Shell32
  Msi)

install(TARGETS krycekium DESTINATION bin)

if(BELAUTILS_ENABLE_LTO)
  set_property(TARGET krycekium PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
