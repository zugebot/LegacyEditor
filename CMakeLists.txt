cmake_minimum_required(VERSION 3.23.1)
project(LegacyEditor)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

# include source directory for imports
include_directories(${CMAKE_SOURCE_DIR}/)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

# compiler flags
if (WIN32 AND NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic -Wno-unused-function")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

# /LegacyEditor/ folder
set(LCEDIT_ASSETS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LegacyEditor/assets)
set(LCEDIT_ASSETS_DEST_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/LegacyEditor)
file(GLOB_RECURSE LCEDIT_ASSETS_FILES "${LCEDIT_ASSETS_SOURCE_DIR}/*")
add_custom_command(
        OUTPUT ${LCEDIT_ASSETS_DEST_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LCEDIT_ASSETS_DEST_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${LCEDIT_ASSETS_SOURCE_DIR} ${LCEDIT_ASSETS_DEST_DIR}
        DEPENDS ${LCEDIT_ASSETS_FILES}
        COMMENT "Copying LegacyEditor assets to build folder"
)

# /lce/ folder
set(LCE_ASSETS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/lce/assets)
set(LCE_ASSETS_DEST_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/lce)
file(GLOB_RECURSE LCE_ASSETS_FILES "${LCE_ASSETS_SOURCE_DIR}/*")
add_custom_command(
        OUTPUT ${LCE_ASSETS_DEST_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LCE_ASSETS_DEST_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${LCE_ASSETS_SOURCE_DIR} ${LCE_ASSETS_DEST_DIR}
        DEPENDS ${LCE_ASSETS_FILES}
        COMMENT "Copying lce assets to build folder"
)

add_custom_target(copy_assets ALL DEPENDS ${LCEDIT_ASSETS_DEST_DIR} ${LCE_ASSETS_DEST_DIR})

file(GLOB_RECURSE LCEDIT_SOURCES
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.cpp"
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.hpp"
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.c"
        "${CMAKE_SOURCE_DIR}/LegacyEditor/*.h"
)

file(GLOB_RECURSE LCE_SOURCES
        "${CMAKE_SOURCE_DIR}/lce/*.cpp"
        "${CMAKE_SOURCE_DIR}/lce/*.hpp"
        "${CMAKE_SOURCE_DIR}/lce/*.c"
        "${CMAKE_SOURCE_DIR}/lce/*.h"
)

file(GLOB_RECURSE INCLUDE_SOURCES
        "${CMAKE_SOURCE_DIR}/include/*.cpp"
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.c"
        "${CMAKE_SOURCE_DIR}/include/*.h"
)

# define executable
add_executable(LegacyEditor
        ${LCEDIT_SOURCES}
        ${LCE_SOURCES}
        ${INCLUDE_SOURCES}
        # examples/figure_out_rpcs3_1_00.cpp
        # examples/readvita.cpp
        tests/batch_convert.cpp
        # examples/write_sfo_from_scratch.cpp
        # examples/figure_out_ps3_to_wiiu.cpp
)

add_dependencies(LegacyEditor copy_assets)