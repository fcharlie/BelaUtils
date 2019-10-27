# BelaUtils cmake
cmake_minimum_required(VERSION 3.12)

project(BelaUtils CXX C)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with BelaUtils.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

option(ENABLE_XAMLDESKTOP "Enable XamlDesktop" OFF)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(BELAUTILS_VERSION_MAJOR 1)
set(BELAUTILS_VERSION_MINOR 0)
set(BELAUTILS_VERSION_PATCH 0)
set(PACKAGE_VERSION "${BELAUTILS_VERSION_MAJOR}.${BELAUTILS_VERSION_MINOR}.${BELAUTILS_VERSION_PATCH}")


set(CPACK_PACKAGE_NAME "BelaUtils")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "BelaUtils")
set(CPACK_PACKAGE_VERSION_MAJOR ${BELAUTILS_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BELAUTILS_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BELAUTILS_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_PACKAGE_VENDOR "BelaUtils")
set(CPACK_PACKAGE_CONTACT "Force Charlie")

include(CPack)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
  message(STATUS "IPO/LTO enabled")
else()
  message(STATUS "IPO/LTO not supported: <${error}>")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -c65001")
add_definitions(
  -D_UNICODE=1
  -DUNICODE=1
)

include_directories(
  ./include
  ./vendor/bela/include
)

add_subdirectory(vendor/rhash)
add_subdirectory(vendor/blake2)
add_subdirectory(vendor/bela)
add_subdirectory(lib/hashlib)
add_subdirectory(lib/winutils)
add_subdirectory(tools/caelum)
add_subdirectory(tools/kisasum)
add_subdirectory(tools/krycekium)
if(ENABLE_XAMLDESKTOP)
  add_subdirectory(tools/xamldesktop)
endif()
