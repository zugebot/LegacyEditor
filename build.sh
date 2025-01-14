#!/bin/bash

# The purpose of this file is for Linux users.

# Set the project directory and build directory
PROJECT_DIR=$(dirname "$(realpath "$0")")
BUILD_DIR=$PROJECT_DIR/cmake-build-release

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

# Change to the build directory
cd "$BUILD_DIR"

# Run CMake to configure the project
cmake ..

# Build the project
cmake --build . --config Release

# End the shell script
echo "Build completed!"