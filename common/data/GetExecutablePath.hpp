#pragma once

#include <string>
#include <iostream>

#include "common/data/ghc/fs_std.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __EMSCRIPTEN__
    // optional, only if you want to allow compile-time override
    #ifndef LEGACYEDITOR_EM_FS_ROOT
        #define LEGACYEDITOR_EM_FS_ROOT "/persist"
    #endif
#endif

class ExecutablePath {
public:
    static fs::path getExecutableDir() {
#ifdef __EMSCRIPTEN__
        return fs::path(LEGACYEDITOR_EM_FS_ROOT);
#elif defined(_WIN32)
        return getWindowsExeDir();
#elif defined(__APPLE__)
        return getMacExeDir();
#elif defined(__linux__)
        return getLinuxExeDir();
#else
        static_assert(false, "Unsupported platform");
#endif
    }

private:
#ifdef defined(_WIN32)
    static fs::path getWindowsExeDir() {
        char buffer[MAX_PATH];
        DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
        if (length == 0) throw std::runtime_error("Failed to get executable path");
        return fs::path(buffer).parent_path();
    }
#endif

#ifdef defined(__linux__)
    static fs::path getLinuxExeDir() {
        char buffer[PATH_MAX];
        ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
        if (length == -1) throw std::runtime_error("Failed to get executable path");
        buffer[length] = '\0';
        return fs::path(buffer).parent_path();
    }
#endif

#ifdef defined(__APPLE__)
    static fs::path getMacExeDir() {
        char buffer[PATH_MAX];
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) != 0)
            throw std::runtime_error("Buffer too small for executable path");
        return fs::path(buffer).parent_path();
    }
#endif
};
