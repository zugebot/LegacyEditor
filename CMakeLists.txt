cmake_minimum_required(VERSION 3.23)

# Pick Ninja
if (NOT CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "" FORCE)
endif()

# Compiler cache
find_program(CCACHE_PROGRAM ccache)
find_program(SCCACHE_PROGRAM sccache)
if (CCACHE_PROGRAM)
    message(STATUS "✔ Using ccache")
    set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
elseif (SCCACHE_PROGRAM)
    message(STATUS "✔ Using sccache")
    set(CMAKE_C_COMPILER_LAUNCHER   "${SCCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
endif()

# Fast linker
find_program(LLD_EXE ld.lld
        PATHS
        "C:/msys64/mingw64/bin"
        NO_DEFAULT_PATH
)
if (LLD_EXE)
    set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> -fuse-ld=lld <CMAKE_CXX_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
    message(STATUS "✔ Forcing lld linker via -fuse-ld=lld (${LLD_EXE})")
else()
    message(WARNING "✘ lld not found, default linker will be used")
endif()


# Parallel builds for plain make
if (CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    include(ProcessorCount)
    ProcessorCount(NPROC)
    set(CMAKE_JOB_POOLS_COMPILE compile_pool=${NPROC})
    set_property(GLOBAL PROPERTY JOB_POOLS compile_pool=${NPROC})
endif()

# Project wide settings
project(LegacyEditor LANGUAGES C CXX)

set(CMAKE_C_STANDARD            17)
set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS       OFF)


# Better optimization in Release, bake in STD libraries as well
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "Flags for Release" FORCE)
set(CMAKE_C_FLAGS_RELEASE   "-O3 -DNDEBUG" CACHE STRING "Flags for Release" FORCE)
if(MSVC)
    foreach(flag_var CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE)
        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static-libgcc -static-libstdc++")
endif()


# set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
# set(CMAKE_CXX_FLAGS_DEBUG "-g1")

# Where final binaries go
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

# Add lce subdirectory
add_subdirectory(include/lce)

# Asset Copy
set(LCEDIT_ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(LCEDIT_ASSETS_DEST_DIR   "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/LegacyEditor")

add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LCEDIT_ASSETS_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${LCEDIT_ASSETS_SOURCE_DIR}" "${LCEDIT_ASSETS_DEST_DIR}"
        COMMENT "Copying assets → ${LCEDIT_ASSETS_DEST_DIR}"
)

# directories to include
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/include/)

# compiler screaming
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)



unset(INCLUDE_OPENSSL CACHE)
option(INCLUDE_OPENSSL "include openssl" OFF)
if(LC_BUILD_EXECUTABLES)
    set(OPENSSL_ROOT_DIR       "C:/msys64/mingw64")
    set(OPENSSL_INCLUDE_DIR    "C:/msys64/mingw64/include")
    set(OPENSSL_CRYPTO_LIBRARY "C:/msys64/mingw64/lib/libcrypto.a")
    find_package(OpenSSL REQUIRED)
endif()




# Glob all sources
file(GLOB_RECURSE LEGACY_EDITOR_SOURCES
        "${CMAKE_SOURCE_DIR}/code/*.cpp"
        "${CMAKE_SOURCE_DIR}/code/*.hpp"
        "${CMAKE_SOURCE_DIR}/common/*.cpp"
        "${CMAKE_SOURCE_DIR}/common/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.cpp"
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.c"
        "${CMAKE_SOURCE_DIR}/include/*.h"
)

# Add executables directly without static lib
set(ALL_SOURCES ${LEGACY_EDITOR_SOURCES} ${LCE_SOURCES})

# Apply to each executable
function(add_cli NAME SRC)
    add_executable(${NAME} "${SRC}" ${ALL_SOURCES})
    target_include_directories(${NAME} PRIVATE ${LCE_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
    target_compile_options(${NAME} PRIVATE ${LCE_COMPILE_OPTIONS})
    target_compile_definitions(${NAME} PRIVATE ${LCE_COMPILE_DEFINITIONS})
    if (DEFINED LCE_PCH)
        target_precompile_headers(${NAME} PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:${LCE_PCH}>")
    endif()
    if (INCLUDE_OPENSSL)
        target_link_libraries(${NAME} PRIVATE ${OPENSSL_CRYPTO_LIBRARY})
    endif()
    add_custom_command(
            TARGET ${NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${LCE_ASSETS_DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${LCE_ASSETS_SOURCE_DIR}" "${LCE_ASSETS_DEST_DIR}"
            COMMENT "Copying lce assets → ${LCE_ASSETS_DEST_DIR}"
    )
endfunction()


unset(LC_BUILD_EXECUTABLES CACHE)
option(LC_BUILD_EXECUTABLES "Build LegacyEditor standalone executable" ON)
if(LC_BUILD_EXECUTABLES)
    add_cli(BatchConverter  tests/batch_convert.cpp)
    add_cli(BatchVersioner  tests/batch_versioner.cpp)
    add_cli(GetPS3SecureID  tests/getPS3SecureID.cpp)
endif()
