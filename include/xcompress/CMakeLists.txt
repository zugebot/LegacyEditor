cmake_minimum_required(VERSION 3.24)
project(xcompress-prebuilt LANGUAGES C)

# Don’t redefine if already present
if (TARGET XCOMPRESS::XCOMPRESS)
    return()
endif()

# Match parent option name
option(USE_STATIC_LZX "Link xcompress statically" OFF)

# Always resolve from source tree (not build dir)
set(_root "${CMAKE_CURRENT_SOURCE_DIR}")
set(_inc  "${_root}/include")

# Layout
if (WIN32)
    if (MSVC)
        set(_bin    "${_root}/bin/MSVC")
        set(_lib    "${_root}/lib/MSVC")
        set(_dll    "${_bin}/xcompress.dll")
        set(_implib "${_lib}/xcompress.lib")
        set(_static "${_lib}/xcompress_static.lib")
    else() # MinGW
        set(_bin    "${_root}/bin/Win32")
        set(_lib    "${_root}/lib/Win32")
        set(_dll    "${_bin}/libxcompress.dll")
        set(_implib "${_lib}/libxcompress.dll.a")
        set(_static "${_lib}/libxcompress.a")
    endif()
elseif (UNIX)
    set(_bin    "${_root}/bin/Unix")
    set(_lib    "${_root}/lib/Unix")
    set(_dll    "${_bin}/libxcompress.so")
    set(_static "${_lib}/libxcompress.a")
else()
    message(FATAL_ERROR "xcompress: unsupported platform")
endif()

# Minimal sanity
if (NOT EXISTS "${_inc}/xdecompress.h")
    message(FATAL_ERROR "xcompress: headers not found at ${_inc}")
endif()

# Pick static or shared by option; just check existence
set(_use_static ${USE_STATIC_LZX})
if (_use_static)
    if (NOT EXISTS "${_static}")
        message(FATAL_ERROR "xcompress: static library not found: ${_static}")
    endif()
else()
    if (NOT EXISTS "${_dll}")
        message(FATAL_ERROR "xcompress: shared runtime not found: ${_dll}")
    endif()
    if (WIN32 AND NOT EXISTS "${_implib}")
        message(FATAL_ERROR "xcompress: import library not found: ${_implib}")
    endif()
endif()

# Imported targets
add_library(XCOMPRESS::XCOMPRESS_static STATIC IMPORTED GLOBAL)
set_target_properties(XCOMPRESS::XCOMPRESS_static PROPERTIES
        IMPORTED_LOCATION "${_static}"
        INTERFACE_INCLUDE_DIRECTORIES "${_inc}"
)

add_library(XCOMPRESS::XCOMPRESS_shared SHARED IMPORTED GLOBAL)
set_target_properties(XCOMPRESS::XCOMPRESS_shared PROPERTIES
        IMPORTED_LOCATION "${_dll}"
        INTERFACE_INCLUDE_DIRECTORIES "${_inc}"
)
if (WIN32)
    set_target_properties(XCOMPRESS::XCOMPRESS_shared PROPERTIES
            IMPORTED_IMPLIB "${_implib}")
endif()

# Unified alias
add_library(XCOMPRESS::XCOMPRESS INTERFACE IMPORTED GLOBAL)
target_link_libraries(XCOMPRESS::XCOMPRESS INTERFACE
        $<IF:$<BOOL:${_use_static}>,XCOMPRESS::XCOMPRESS_static,XCOMPRESS::XCOMPRESS_shared>
)

# Your headers switch on LZX_STATIC for static link on Windows
if (WIN32 AND _use_static)
    target_compile_definitions(XCOMPRESS::XCOMPRESS INTERFACE LZX_STATIC=1)
endif()

# Expose DLL path for staging in parent (when shared)
set(LZX_RUNTIME "${_dll}" PARENT_SCOPE)
