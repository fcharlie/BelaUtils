# misc libs

add_library(baulk.misc STATIC chardet.cc hash.cc indicators.cc)

target_include_directories(baulk.misc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ced)

target_link_libraries(baulk.misc ced belawin belahash)
