cmake_minimum_required(VERSION 3.29.6)

# Prefer Ninja if not specified
if (NOT CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "" FORCE)
endif()

# Project wide settings
project(LegacyEditor LANGUAGES C CXX)

set(CMAKE_C_STANDARD            17)
set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS       OFF)

unset(INCLUDE_OPENSSL CACHE)
unset(LC_BUILD_EXECUTABLES CACHE)
unset(USE_LLD_LINKING CACHE)
unset(STATIC_GNU_RUNTIME CACHE)


add_compile_definitions(MEMSET_ZERO)
option(INCLUDE_OPENSSL "include openssl" OFF)
option(STATIC_GNU_RUNTIME "Statically link libstdc++/libgcc on MinGW/Clang" OFF)
option(LC_BUILD_EXECUTABLES "Build LegacyEditor standalone executable" ON)
option(USE_LLD_LINKING "fortnite" OFF)

# -------------------------------
# C/C++ cache (ccache/sccache)
# -------------------------------
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


# -------------------------------
# Windows resource (version info)
# -------------------------------
if (WIN32)
    enable_language(RC)
    set(LE_VERSION_RC "${CMAKE_SOURCE_DIR}/res/version.rc")
endif()


# -------------------------------
# Build flags
# -------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

# Release opts (keep reasonable)
set(CMAKE_C_FLAGS_RELEASE   "-O3 -DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)


# Prefer using the default link rule; only *add* options (don’t override CMAKE_CXX_LINK_EXECUTABLE)
# If you really want lld, add it as a link option instead of overriding the whole rule.
if (WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    find_program(LLD_EXE ld.lld
            PATHS
            "C:/msys64/mingw64/bin"
            NO_DEFAULT_PATH
    )
    if (LLD_EXE AND USE_LLD_LINKING)
        message(STATUS "✔ Using lld via -fuse-ld=lld (${LLD_EXE})")
        add_link_options(-fuse-ld=lld)
    endif()

    # Hardened PE flags on MinGW/Clang
    add_link_options(-Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va)

    if (STATIC_GNU_RUNTIME)
        message(WARNING "Linking libstdc++/libgcc statically (can increase AV false-positives).")
        add_link_options(-static-libgcc -static-libstdc++)
    endif()
endif()

if (MSVC)
    add_link_options($<$<CONFIG:Release>:/INCREMENTAL:NO> /DYNAMICBASE /NXCOMPAT /MANIFEST:EMBED)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_link_options(/CETCOMPAT)
    endif()
endif()


# Parallel builds for plain make
if (CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    include(ProcessorCount)
    ProcessorCount(NPROC)
    set(CMAKE_JOB_POOLS_COMPILE compile_pool=${NPROC})
    set_property(GLOBAL PROPERTY JOB_POOLS compile_pool=${NPROC})
endif()


# -------------------------------
# Assets
# -------------------------------
set(LCEDIT_ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(LCEDIT_ASSETS_DEST_DIR   "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/LegacyEditor")
add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LCEDIT_ASSETS_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${LCEDIT_ASSETS_SOURCE_DIR}" "${LCEDIT_ASSETS_DEST_DIR}"
        COMMENT "Copying assets → ${LCEDIT_ASSETS_DEST_DIR}"
)


# -------------------------------
# Include dirs & defines
# -------------------------------
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/include/)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)


if(INCLUDE_OPENSSL)
    set(OPENSSL_ROOT_DIR       "C:/msys64/mingw64")
    set(OPENSSL_INCLUDE_DIR    "C:/msys64/mingw64/include")
    set(OPENSSL_CRYPTO_LIBRARY "C:/msys64/mingw64/lib/libcrypto.a")
    find_package(OpenSSL REQUIRED)
endif()


# -------------------------------
# Sources
# -------------------------------
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


# Bring in lce
add_subdirectory(include/lce)
set(ALL_SOURCES ${LEGACY_EDITOR_SOURCES} ${LCE_SOURCES})


# Apply to each executable
function(add_cli NAME SRC)
    add_executable(${NAME} "${SRC}" ${ALL_SOURCES})
    if (WIN32 AND DEFINED LE_VERSION_RC AND EXISTS "${LE_VERSION_RC}")
        target_sources(${NAME} PRIVATE "${LE_VERSION_RC}")
    endif()
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
    if (MINGW AND STATIC_GNU_RUNTIME)
        target_link_options(${NAME} PRIVATE -static-libgcc -static-libstdc++)
        # Map pthreads correctly (MinGW uses winpthread)
        find_package(Threads)
        if (Threads_FOUND)
            target_link_libraries(${NAME} PRIVATE Threads::Threads)
        endif()
        # OPTIONAL: also static winpthread (needs mingw-w64-x86_64-winpthreads-static installed)
        # target_link_options(${NAME} PRIVATE -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic)
    endif()
endfunction()


if(LC_BUILD_EXECUTABLES)
    add_cli(BatchConverter        tests/batch_convert.cpp)
    add_cli(BatchVersioner        tests/batch_versioner.cpp)
    add_cli(GetPS3SecureID        tests/getPS3SecureID.cpp)
    add_cli(ChunkV11Testing       tests/get_chunkV11_working.cpp)
    add_cli(TestGRF               tests/testGRF.cpp)
    add_cli(BruteForceCRC         tests/crc_bruteforcer.cpp)
    add_cli(RleNSXPS4Decompress   tests/rleNSXPS4_decompress.cpp)
    add_cli(ensureEntitiesCorrect tests/ensureEntitiesReadWriteSame.cpp)
    add_cli(SFOPlayground         tests/write_sfo_from_scratch.cpp)
endif()
