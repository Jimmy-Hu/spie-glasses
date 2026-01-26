#!/bin/bash

# ==============================================================================
# Build Script for WSL (Windows Subsystem for Linux) - Ubuntu
# ==============================================================================

# Exit immediately if a command exits with a non-zero status
set -e

echo ">>> Starting build process for WSL..."

# ------------------------------------------------------------------------------
# 1. Install Dependencies
# ------------------------------------------------------------------------------
echo ">>> Installing dependencies..."

# Check if we have sudo privileges
if [ "$EUID" -ne 0 ]; then
  SUDO="sudo"
else
  SUDO=""
fi

# Update package list
$SUDO apt-get update

# Install required libraries based on CMakeLists.txt dependencies
# - build-essential: Compiler (gcc/g++) and make
# - cmake: Build system
# - libboost-all-dev: Boost libraries (System, etc.)
# - libfmt-dev: fmt library
# - libmsgpack-dev: MessagePack library
# - libopencv-dev: OpenCV library
# - nlohmann-json3-dev: JSON library
# - zlib1g-dev: ZLIB compression
# - liblz4-dev: LZ4 compression
# - libdlib-dev: Dlib machine learning library
# - libx11-dev: Required for Dlib GUI support
# - libblas-dev/liblapack-dev/libopenblas-dev: Linear algebra for Dlib
$SUDO apt-get install -y \
    build-essential \
    cmake \
    libboost-all-dev \
    libfmt-dev \
    libmsgpack-dev \
    libopencv-dev \
    nlohmann-json3-dev \
    zlib1g-dev \
    liblz4-dev \
    libdlib-dev \
    libx11-dev \
    libblas-dev \
    liblapack-dev \
    libopenblas-dev

echo ">>> Dependencies installed successfully."

# ------------------------------------------------------------------------------
# 2. Configure CMake
# ------------------------------------------------------------------------------
echo ">>> Configuring CMake..."

# Define build directory
BUILD_DIR="build"

# Remove existing build directory to ensure a clean build (Optional)
# rm -rf "$BUILD_DIR"

# Generate build system
# -B: Build directory
# -DCMAKE_BUILD_TYPE=Release: Optimize for release
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release

# ------------------------------------------------------------------------------
# 3. Build Project
# ------------------------------------------------------------------------------
echo ">>> Building project..."

# Determine number of CPU cores for parallel build
CORES=$(nproc)
echo ">>> Using $CORES cores for compilation."

# Build the project
cmake --build "$BUILD_DIR" --config Release --parallel "$CORES"

echo "=============================================================================="
echo ">>> Build completed successfully!"
echo ">>> Executables can be found in: $BUILD_DIR/export/bin/"
echo "=============================================================================="