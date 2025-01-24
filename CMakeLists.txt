cmake_minimum_required(VERSION 3.23.1)
project(LegacyEditor)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")


include_directories(${CMAKE_SOURCE_DIR}/)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)






# compiler flags
if (WIN32 AND NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic -Wno-unused-function")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()



# /LegacyEditor/assets folder
set(LCEDIT_ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(LCEDIT_ASSETS_DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/LegacyEditor")

# /lce/assets folder
set(LCE_ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/include/lce/assets")
set(LCE_ASSETS_DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/lce")

# Single custom target to copy both asset directories
add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LCEDIT_ASSETS_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${LCEDIT_ASSETS_SOURCE_DIR}" "${LCEDIT_ASSETS_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LCE_ASSETS_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${LCE_ASSETS_SOURCE_DIR}" "${LCE_ASSETS_DEST_DIR}"
        COMMENT "Copying all asset folders to the build directory"
)

file(GLOB_RECURSE LCEDIT_SOURCES
        "${CMAKE_SOURCE_DIR}/code/*.cpp"
        "${CMAKE_SOURCE_DIR}/code/*.hpp"

        "${CMAKE_SOURCE_DIR}/common/*.*pp"
        "${CMAKE_SOURCE_DIR}/common/*.hpp"

        "${CMAKE_SOURCE_DIR}/include/*.*pp"
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.c"
        "${CMAKE_SOURCE_DIR}/include/*.h"
)



# define executable
add_executable(LegacyEditor
        ${LCEDIT_SOURCES}
        # examples/figure_out_rpcs3_1_00.cpp
        # examples/readvita.cpp
        tests/batch_convert.cpp
        # examples/write_sfo_from_scratch.cpp
        # examples/figure_out_ps3_to_wiiu.cpp
)

add_dependencies(LegacyEditor copy_assets)