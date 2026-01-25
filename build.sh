#!/bin/bash

# ==============================================================================
# Build Script for spie-glasses
# Automatically handles dependency installation and source code patching
# based on the project's CI configuration.
# ==============================================================================

set -e # Exit immediately if a command exits with a non-zero status.

# Detect OS
OS="$(uname -s)"
BUILD_DIR="build"
BUILD_TYPE="Release"

echo "Detected OS: $OS"

# ------------------------------------------------------------------------------
# 1. Install Dependencies
# ------------------------------------------------------------------------------
echo ">>> Step 1: Checking/Installing Dependencies..."

if [ "$OS" == "Linux" ]; then
    # Check if apt-get is available (Debian/Ubuntu)
    if command -v apt-get &> /dev/null; then
        echo "Installing dependencies via apt-get (requires sudo)..."
        sudo apt-get update
        sudo apt-get install -y \
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
    else
        echo "Warning: apt-get not found. Please ensure dependencies are installed manually."
    fi

elif [ "$OS" == "Darwin" ]; then
    # Check if Homebrew is available
    if command -v brew &> /dev/null; then
        echo "Installing dependencies via Homebrew..."
        brew update
        # Install packages individually to avoid failure if some are already installed
        for pkg in cmake boost fmt msgpack-cxx opencv nlohmann-json zlib lz4 dlib openblas; do
            brew install $pkg || brew upgrade $pkg || true
        done
    else
        echo "Error: Homebrew is required for automatic dependency installation on macOS."
        echo "Please install Homebrew from https://brew.sh/ or install dependencies manually."
        exit 1
    fi
else
    echo "Unsupported OS: $OS. Please install dependencies manually."
fi

# ------------------------------------------------------------------------------
# 2. Patch Source Code (Consistent with CI)
# ------------------------------------------------------------------------------
echo ">>> Step 2: Patching Source Code for Compatibility..."

# Helper function for cross-platform sed (BSD sed vs GNU sed)
run_sed() {
    # On macOS, sed -i requires an extension argument (e.g., -i .bak)
    # On Linux, standard sed -i works, but -i.bak is also supported for backup
    sed -i.bak "$@"
}

# --- Fix 1: Boost.Asio io_context compatibility ---
echo "Applying Boost.Asio patches..."

# A. Replace deprecated '.post()' calls
# asio_msgpack.hpp
if [ -f "include/api/asio_msgpack.hpp" ]; then
    run_sed 's/s.get_io_service().post(/boost::asio::post(s.get_executor(), /g' include/api/asio_msgpack.hpp
fi

# session.hpp
if [ -f "src/server/session.hpp" ]; then
    run_sed 's/m_socket.get_io_service().post(/boost::asio::post(m_socket.get_executor(), /g' src/server/session.hpp
fi

# server.hpp
if [ -f "src/server/server.hpp" ]; then
    run_sed 's/m_acceptor.get_io_service().post(/boost::asio::post(m_acceptor.get_executor(), /g' src/server/server.hpp
fi

# queued_writer.hpp
if [ -f "include/api/queued_writer.hpp" ]; then
    # Critical Fix: Use .get_io_service().get_executor()
    run_sed 's/m_writer.get_io_service().post(/boost::asio::post(m_writer.get_io_service().get_executor(), /g' include/api/queued_writer.hpp
fi

# B. Fix get_io_service() implementation in headers
# basic_msgpack_writer.hpp
if [ -f "include/api/basic_msgpack_writer.hpp" ]; then
    run_sed 's/m_ws.get_io_service()/((boost::asio::io_context\&)m_ws.get_executor().context())/g' include/api/basic_msgpack_writer.hpp
fi

# basic_msgpack_reader.hpp
if [ -f "include/api/basic_msgpack_reader.hpp" ]; then
    run_sed 's/m_rs.get_io_service()/((boost::asio::io_context\&)m_rs.get_executor().context())/g' include/api/basic_msgpack_reader.hpp
fi

# --- Fix 2: OpenCV 4.x compatibility ---
echo "Applying OpenCV 4.x patches..."
if [ -f "src/api/Detect.cpp" ]; then
    run_sed 's/CV_BGR2HSV/cv::COLOR_BGR2HSV/g' src/api/Detect.cpp
    run_sed 's/CV_HSV2BGR/cv::COLOR_HSV2BGR/g' src/api/Detect.cpp
fi

# --- Fix 3: Dlib System Header Patch (Linux & macOS) ---
echo "Checking Dlib GUI support configuration..."
DLIB_HEADER=""

if [ "$OS" == "Linux" ]; then
    DLIB_HEADER="/usr/include/dlib/gui_core/gui_core_kernel_2.h"
elif [ "$OS" == "Darwin" ]; then
    # On macOS, find path via brew
    if command -v brew &> /dev/null; then
        DLIB_PREFIX="$(brew --prefix dlib)"
        DLIB_HEADER="$DLIB_PREFIX/include/dlib/gui_core/gui_core_kernel_2.h"
    fi
fi

if [ -n "$DLIB_HEADER" ] && [ -f "$DLIB_HEADER" ]; then
    # Check if the undef line already exists
    if ! grep -q "#undef DLIB_NO_GUI_SUPPORT" "$DLIB_HEADER"; then
        echo "Patching Dlib header at $DLIB_HEADER to enable GUI support..."
        
        # Check permissions and use sudo if necessary
        if [ -w "$DLIB_HEADER" ]; then
            sed -i.bak '1i #undef DLIB_NO_GUI_SUPPORT' "$DLIB_HEADER"
        else
            echo "Requesting sudo permission to patch system header..."
            sudo sed -i.bak '1i #undef DLIB_NO_GUI_SUPPORT' "$DLIB_HEADER"
        fi
    else
        echo "Dlib header already patched."
    fi
else
    echo "Warning: Dlib header file not found at expected location ($DLIB_HEADER). Skipping patch."
fi

# Cleanup backup files
echo "Cleaning up backup (.bak) files..."
find . -name "*.bak" -type f -delete

# ------------------------------------------------------------------------------
# 3. Configure and Build
# ------------------------------------------------------------------------------
echo ">>> Step 3: Configuring CMake..."

# Define CMake arguments
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

# Add macOS specific arguments
if [ "$OS" == "Darwin" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBoost_NO_BOOST_CMAKE=ON -Wno-dev"
fi

# Generate Build System
cmake -B "$BUILD_DIR" $CMAKE_ARGS

echo ">>> Step 4: Building Project..."
# Build
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "=============================================================================="
echo "Build Complete Successfully!"
echo "Executables are located in: $BUILD_DIR/export/bin/"
echo "=============================================================================="