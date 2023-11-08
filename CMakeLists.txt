cmake_minimum_required(VERSION 3.24)
project(LegacyEditor)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/)
file(GLOB_RECURSE LEGACY_EDITOR_SOURCES
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.cpp"
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.hpp"
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.c"
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.h"
)

add_executable(LegacyEditor main.cpp ${LEGACY_EDITOR_SOURCES} LegacyEditor/utils/NBT/include.hpp LegacyEditor/LCE/SaveFile/BINSupport.cpp)
