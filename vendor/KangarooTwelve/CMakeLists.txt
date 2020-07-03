#
string(TOLOWER "${CMAKE_C_COMPILER_ARCHITECTURE_ID}" K12_ARCH)

if("${K12_ARCH}" STREQUAL "x86_64" OR "${K12_ARCH}" STREQUAL "amd64" OR "${K12_ARCH}" STREQUAL "x64")
  set(K12_SOURCE
    Optimized64/KeccakP-1600-opt64.c
    Optimized64/KeccakP-1600-AVX512-plainC.c
    Optimized64/KeccakP-1600-timesN-SSSE3.c
    Optimized64/KeccakP-1600-timesN-AVX2.c
    Optimized64/KeccakP-1600-timesN-AVX512.c
    Optimized64/KeccakP-1600-runtimeDispatch.c
  )
  set(K12_INC Optimized64)
  set_source_files_properties(Optimized64/KeccakP-1600-timesN-AVX512.c PROPERTIES COMPILE_FLAGS "-arch:AVX512")
elseif("${K12_ARCH}" STREQUAL "arm64")
 set(K12_SOURCE Optimized64/KeccakP-1600-opt64.c Plain64/KeccakP-1600-plain64.c)
 set(K12_INC Plain64)
else()
 set(K12_SOURCE Inplace32BI/KeccakP-1600-inplace32BI.c)
 set(K12_INC Inplace32BI)
endif()

add_library(KangarooTwelve STATIC KangarooTwelve.c ${K12_SOURCE})

target_include_directories(KangarooTwelve PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${K12_INC})
target_compile_definitions(KangarooTwelve PRIVATE KeccakP1600_noAssembly)