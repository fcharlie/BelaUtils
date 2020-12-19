#

add_executable(
  bona
  bona.cc
  bona.elf.cc
  bona.macho.cc
  bona.pe.cc
  bona.zip.cc
  bona.rc
  bona.manifest)

target_link_libraries(
  bona
  belawin
  belatime
  belaund
  hazel)

install(TARGETS bona DESTINATION bin)