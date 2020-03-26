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
  belash
  d2d1
  dwrite
  Shell32
  Msi)
