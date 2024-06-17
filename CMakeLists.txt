cmake_minimum_required(VERSION 3.24)
project(LegacyEditor)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# remove this if you are building your own project
# add_definitions(-DUNIT_TESTS)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

include_directories(${CMAKE_SOURCE_DIR}/)

file(GLOB_RECURSE LEGACY_EDITOR_SOURCES
    "${CMAKE_SOURCE_DIR}/LegacyEditor/*.cpp"
    "${CMAKE_SOURCE_DIR}/LegacyEditor/*.hpp"
    "${CMAKE_SOURCE_DIR}/LegacyEditor/*.c"
    "${CMAKE_SOURCE_DIR}/LegacyEditor/*.h"
)

file(GLOB_RECURSE INCLUDE_SOURCES
    "${CMAKE_SOURCE_DIR}/include/*.cpp"
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_SOURCE_DIR}/include/*.c"
    "${CMAKE_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE LCE_RESOURCES
    "${CMAKE_SOURCE_DIR}/lce/*.cpp"
    "${CMAKE_SOURCE_DIR}/lce/*.hpp"
    "${CMAKE_SOURCE_DIR}/lce/*.c"
    "${CMAKE_SOURCE_DIR}/lce/*.h"
)

add_executable(LegacyEditor
    ${LEGACY_EDITOR_SOURCES}
    ${LCE_RESOURCES}
    ${INCLUDE_SOURCES}
    examples/trying_ps4_save_render.cpp
)