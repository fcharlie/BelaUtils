# wind
add_executable(wind
  wind.cc
  indicators.cc
  net.cc
  tcp.cc
  wind.rc
  wind.manifest
)

target_link_libraries(wind
  belahash
  belawin
  winhttp
  ws2_32
  Msi
)

install(TARGETS wind DESTINATION bin)