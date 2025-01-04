# BelaUtils cmake
cmake_minimum_required(VERSION 3.22)

project(BelaUtils CXX C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE)
  message(
    FATAL_ERROR
      "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with BelaUtils.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(BELAUTILS_ENABLE_LTO OFF)
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
  message(STATUS "IPO/LTO supported")
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BELAUTILS_ENABLE_LTO ON)
    set(BELA_ENABLE_LTO ON)
    message(STATUS "IPO/LTO enabled")
  endif()
endif()

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS=1 -utf-8 -W3 -DUNICODE=1 -D_UNICODE=1 -wd26812")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS=1 -utf-8 -W3 -permissive- -Zc:__cplusplus -DUNICODE=1 -D_UNICODE=1 -wd26812"
  )
  set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -c65001")
endif(MSVC)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Gen version
include(VersionFromVCS)
get_source_info(${CMAKE_CURRENT_SOURCE_DIR} BELAUTILS_REVISION BELAUTILS_REMOTE_URL BELAUTILS_REFNAME)
string(TIMESTAMP BELAUTILS_COPYRIGHT_YEAR "%Y")

# BELAUTILS_REFNAME maybe defined by ci
if("${BELAUTILS_REFNAME}" STREQUAL "none")
  if(DEFINED ENV{GITHUB_REF_NAME})
    set(BELAUTILS_REFNAME $ENV{GITHUB_REF_NAME})
  endif()
endif()

if("${BELAUTILS_REFNAME}" MATCHES "refs/tags/*")
  string(TIMESTAMP BELAUTILS_BUILD_TIME "%Y-%m-%dT%H:%M:%SZ")
else()
  set(BELAUTILS_BUILD_TIME "none")
endif()

if(DEFINED ENV{GITHUB_RUN_NUMBER})
  set(BELAUTILS_VERSION_BUILD $ENV{GITHUB_RUN_NUMBER})
  message(STATUS "BelaUtils detect GITHUB_RUN_NUMBER: $ENV{GITHUB_RUN_NUMBER}")
else()
  set(BELAUTILS_VERSION_BUILD 261)
endif()

set(BELAUTILS_VERSION_MAJOR 2)
set(BELAUTILS_VERSION_MINOR 0)
set(BELAUTILS_VERSION_PATCH 3)
set(PACKAGE_VERSION "${BELAUTILS_VERSION_MAJOR}.${BELAUTILS_VERSION_MINOR}.${BELAUTILS_VERSION_PATCH}")

string(TOLOWER "${CMAKE_C_COMPILER_ARCHITECTURE_ID}" COMPILER_ARCH_ID)
set(CPACK_SYSTEM_NAME "win-${COMPILER_ARCH_ID}")
set(CPACK_PACKAGE_NAME "BelaUtils")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "BelaUtils")
set(CPACK_PACKAGE_VERSION_MAJOR ${BELAUTILS_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BELAUTILS_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BELAUTILS_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_PACKAGE_VENDOR "BelaUtils")
set(CPACK_PACKAGE_CONTACT "BelaUtils contributors")

include(CPack)
# Generate version code
configure_file(${CMAKE_SOURCE_DIR}/include/belautilsversion.h.cmake ${CMAKE_BINARY_DIR}/include/belautilsversion.h)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories("${CMAKE_BINARY_DIR}/include" ./include ./vendor/bela/include)

add_subdirectory(vendor/blake2)
add_subdirectory(vendor/KangarooTwelve)
add_subdirectory(vendor/bela)
add_subdirectory(lib)
add_subdirectory(tools)
