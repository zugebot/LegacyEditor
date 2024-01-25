cmake_minimum_required(VERSION 3.24)
project(LegacyEditor)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

include_directories(${CMAKE_SOURCE_DIR}/)
file(GLOB_RECURSE LEGACY_EDITOR_SOURCES
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.cpp"
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.hpp"
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.c"
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.h"
)

add_executable(LegacyEditor main.cpp ${LEGACY_EDITOR_SOURCES}
        LegacyEditor/LCE/Chunk/helpers.hpp)
