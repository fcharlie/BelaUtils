#

add_executable(
  caelum WIN32
  caelum.cc
  pecoff.cc
  window.cc
  resolvelink.cc
  caelum.rc
  caelum.manifest)
# no atl depend
target_link_libraries(
  caelum
  belawin
  belashl
  d2d1
  dwrite
  Shell32)

install(TARGETS caelum DESTINATION bin)

if(BELAUTILS_ENABLE_LTO)
  set_property(TARGET caelum PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
