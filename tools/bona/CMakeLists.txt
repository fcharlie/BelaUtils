#

add_executable(
  bona
  bona.cc
  bona.chardet.cc
  bona.elf.cc
  bona.macho.cc
  bona.pe.cc
  bona.zip.cc
  bona.xml.cc
  bona.rc
  bona.manifest)

target_include_directories(bona PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/ced)

target_link_libraries(
  bona
  belawin
  belatime
  belaund
  hazel
  ced)

install(TARGETS bona DESTINATION bin)