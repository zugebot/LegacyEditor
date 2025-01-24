@echo off
setlocal
:: The purpose of this file is for stinky Windows people that use VS code.

:: Set the project directory and build directory
set PROJECT_DIR=%~dp0
set BUILD_DIR=%PROJECT_DIR%\cmake-build-release

:: Create the build directory if it doesn't exist
if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

:: Change to the build directory
cd %BUILD_DIR%

:: Run CMake to configure the project
cmake ..

:: Build the project
cmake --build . --config Release

:: End the batch script
endlocal
pause