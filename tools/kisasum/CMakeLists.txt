# kisasum

add_executable(
  kisasum-ui WIN32
  ui/kisasum.cc
  ui/kisasumutils.cc
  ui/ui.cc
  ui/kisasum.rc
  ui/kisasum.manifest)

target_link_libraries(
  kisasum-ui
  hashlib
  belawin
  belashl
  d2d1
  dwrite
  Shcore
  Shell32)

target_include_directories(kisasum-ui PRIVATE "../../lib/hashlib")

add_executable(kisasum cli/kisasum.cc cli/indicators.cc cli/kisasum.rc cli/kisasum.manifest)

target_link_libraries(kisasum hashlib belawin)

target_include_directories(kisasum PRIVATE "../../lib/hashlib")

install(TARGETS kisasum-ui DESTINATION bin)
install(TARGETS kisasum DESTINATION bin)

if(BELAUTILS_ENABLE_LTO)
  set_property(TARGET kisasum-ui PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  set_property(TARGET kisasum PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
