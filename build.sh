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
# - Additional X11 libs: libxext-dev, libsm-dev, libxrender-dev (Added to fix DLIB_NO_GUI_SUPPORT issue)
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
    libxext-dev \
    libsm-dev \
    libxrender-dev \
    libblas-dev \
    liblapack-dev \
    libopenblas-dev

echo ">>> Dependencies installed successfully."

# ------------------------------------------------------------------------------
# 1.5. Patch Dlib (Force Enable GUI)
# ------------------------------------------------------------------------------
# In some WSL/Ubuntu distributions, the default dlib package has DLIB_NO_GUI_SUPPORT 
# defined by default or in a way that conflicts even when X11 is present.
# We will check for the specific header that throws the error and comment out the check
# or forcefully undefine the macro at the beginning of the file.
DLIB_HEADER="/usr/include/dlib/gui_core/gui_core_kernel_2.h"

if [ -f "$DLIB_HEADER" ]; then
    echo ">>> Patching dlib header to force enable GUI support: $DLIB_HEADER"
    # We use sed to insert '#undef DLIB_NO_GUI_SUPPORT' at the top of the file.
    # This ensures that even if it was defined previously, we disable that definition
    # before the check code runs.
    # We verify if the patch is already applied to avoid duplicate lines.
    if ! grep -q "#undef DLIB_NO_GUI_SUPPORT" "$DLIB_HEADER"; then
        $SUDO sed -i '1i #undef DLIB_NO_GUI_SUPPORT' "$DLIB_HEADER"
        echo ">>> Dlib header patched."
    else
        echo ">>> Dlib header already patched."
    fi
else
    echo ">>> WARNING: Dlib header not found at $DLIB_HEADER. Skipping patch."
fi

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

cp ./src/server/shape_predictor_68_face_landmarks.dat ./build/export/bin/shape_predictor_68_face_landmarks.dat
cp ./src/server/Training_official.dat ./build/export/bin/Training_official.dat

echo "=============================================================================="
echo ">>> Build completed successfully!"
echo ">>> Executables can be found in: $BUILD_DIR/export/bin/"
echo "=============================================================================="








