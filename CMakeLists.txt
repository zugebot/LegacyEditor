cmake_minimum_required(VERSION 3.23.1)
project(LIB_LegacyEditor)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

# directories to include
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/include/)

# compiler screaming
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

# glob all header files
file(GLOB_RECURSE LCEDIT_SOURCES
        "${CMAKE_SOURCE_DIR}/code/*.cpp"
        "${CMAKE_SOURCE_DIR}/code/*.hpp"
        "${CMAKE_SOURCE_DIR}/common/*.cpp"
        "${CMAKE_SOURCE_DIR}/common/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.cpp"
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.c"
        "${CMAKE_SOURCE_DIR}/include/*.h"
)

add_library(LIB_LegacyEditor STATIC ${LCEDIT_SOURCES})
target_compile_options(LIB_LegacyEditor PRIVATE -march=native -DNDEBUG)

target_include_directories(LIB_LegacyEditor
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

add_dependencies(LIB_LegacyEditor copy_assets)
add_compile_definitions(DONT_MEMSET0)



unset(LC_BUILD_EXECUTABLE CACHE)
option(LC_BUILD_EXECUTABLE "Build LegacyCubiomes standalone executable" ON)
if(LC_BUILD_EXECUTABLE)

    add_executable(BatchConverter "tests/batch_convert.cpp")
    target_link_libraries(BatchConverter PRIVATE LIB_LegacyEditor)

    add_executable(BatchVersioner "tests/batch_versioner.cpp")
    target_link_libraries(BatchVersioner PRIVATE LIB_LegacyEditor)

endif()



