#

add_executable(
  belaview
  belaview.cc
  belaview.elf.cc
  belaview.macho.cc
  belaview.pe.cc
  belaview.zip.cc
  belaview.rc
  belaview.manifest)

target_link_libraries(
  belaview
  belawin
  belatime
  belaund
  hazel)

install(TARGETS belaview DESTINATION bin)