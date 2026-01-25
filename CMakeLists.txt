##
## CMakeLists.txt (Rewritten to standard CMake)
##
## Original Author: ChienYu Lin
##
cmake_minimum_required (VERSION 3.5) # Update to 3.5 to meet modern macOS/CMake requirements

# ------------------------------------------------------------------------------
# Hunter Package Manager Setup (Reserved from original settings)
# ------------------------------------------------------------------------------
if (DEFINED USE_HUNTER)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/HunterGate.cmake")
    message(STATUS "'cmake/HunterGate.cmake' not found. Start downloading that file.")
    file(
      DOWNLOAD
      "https://github.com/hunter-packages/gate/raw/master/cmake/HunterGate.cmake"
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/HunterGate.cmake"
    )
  endif()
  include("cmake/HunterGate.cmake")
  HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.18.39.tar.gz"
    SHA1 "a6fbc056c3d9d7acdaa0a07c575c9352951c2f6c"
  )
endif()

# ------------------------------------------------------------------------------
# Helper Functions (Reserved original helper functions)
# ------------------------------------------------------------------------------

function(add_package package)
  if (DEFINED USE_HUNTER)
    hunter_add_package(${package} ${ARGN})
  endif()
endfunction()

function(setup_package package)
  add_package(${package})
  find_package(${package} ${ARGN})
endfunction()

function(show_variables)
  foreach(var ${ARGV})
    message(STATUS "${var}=${${var}}")
  endforeach()
endfunction()

# ------------------------------------------------------------------------------
# Macro: Replaces sweet's config_from_generic_package
# Links legacy FindPackage variables (LIBRARIES, INCLUDES, etc.) to the specified Interface Target
# ------------------------------------------------------------------------------
macro(link_legacy_package_vars target_name package_name)
  if(${package_name}_FOUND OR ${package_name})
    # Handle Libraries
    foreach(suffix LIBRARIES LIBRARY LIBS)
      if (DEFINED ${package_name}_${suffix} AND NOT "${${package_name}_${suffix}}" STREQUAL "")
        target_link_libraries(${target_name} INTERFACE ${${package_name}_${suffix}})
        break()
      endif()
    endforeach()

    # Handle Include Dirs
    foreach(suffix INCLUDES INCLUDE_DIR INCLUDE_DIRS)
      if (DEFINED ${package_name}_${suffix} AND NOT "${${package_name}_${suffix}}" STREQUAL "")
        target_include_directories(${target_name} INTERFACE ${${package_name}_${suffix}})
        break()
      endif()
    endforeach()

    # Handle Definitions
    foreach(suffix DEFINITIONS)
      if (DEFINED ${package_name}_${suffix} AND NOT "${${package_name}_${suffix}}" STREQUAL "")
        target_compile_definitions(${target_name} INTERFACE ${${package_name}_${suffix}})
        break()
      endif()
    endforeach()
  endif()
endmacro()

# ------------------------------------------------------------------------------
# Project Settings
# ------------------------------------------------------------------------------
project(spie-glass) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/export/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/export/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/export/lib")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

set(CMAKE_C_STANDARD              90)
set(CMAKE_C_STANDARD_REQUIRED     ON)
set(CMAKE_CXX_STANDARD            14)
set(CMAKE_CXX_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_FLAGS "-O2")

# ------------------------------------------------------------------------------
# Find Packages
# ------------------------------------------------------------------------------

# Common Packages
find_package(fmt CONFIG REQUIRED)

# [Platform Specific Logic]
if(APPLE)
  # --- macOS (Homebrew) ---
  # Homebrew packages are usually dynamic libs (.dylib) or Header-only.
  set(Boost_USE_STATIC_LIBS OFF)
  set(Msgpack_STATIC OFF)
  set(OpenCV_STATIC OFF)

  # Boost 1.90+ on macOS no longer defines io_service, manually map it to io_context
  add_definitions(-Dio_service=io_context)
  
  # Boost: Find Headers first, System component is Optional (might be Header-only)
  find_package(Boost REQUIRED)
  find_package(Boost OPTIONAL_COMPONENTS system)

  # Msgpack: Try to find package, if failed manually search for Header (Homebrew msgpack-cxx is Header-only)
  find_package(Msgpack QUIET)
  if(NOT Msgpack_FOUND)
     find_path(Msgpack_INCLUDE_DIRS NAMES msgpack.hpp HINTS /opt/homebrew/include /usr/local/include)
     if(Msgpack_INCLUDE_DIRS)
        set(Msgpack_FOUND TRUE)
        # Explicitly clear LIBRARIES to prevent linker errors with "Msgpack_LIBRARIES-NOTFOUND"
        set(Msgpack_LIBRARIES "")
        message(STATUS "Found Msgpack headers (Header-only): ${Msgpack_INCLUDE_DIRS}")
     else()
        message(FATAL_ERROR "Could NOT find Msgpack.")
     endif()
  endif()

  # OpenCV: Use default (Dynamic)
  find_package(OpenCV REQUIRED)

else()
  # --- Linux (Ubuntu/Debian) ---
  # Usually has static libs (.a) and System exists as a library
  set(Boost_USE_STATIC_LIBS ON)
  set(Msgpack_STATIC ON)
  set(OpenCV_STATIC ON)
  
  # Boost
  set(Boost_COMPONENTS system)
  find_package(Boost REQUIRED COMPONENTS ${Boost_COMPONENTS})
  
  # Msgpack
  find_package(Msgpack REQUIRED)
  
  # OpenCV
  find_package(OpenCV REQUIRED)
endif()

# Other Common Packages
find_package(Json REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LZ4 REQUIRED)
find_package(dlib REQUIRED)

set(packages Boost fmt Msgpack OpenCV ZLIB dlib Json LZ4)

# ------------------------------------------------------------------------------
# Centralized 3rd-party Dependencies (Replaces sweet_config)
# Creates an Interface Library to centrally manage all 3rd-party dependencies
# ------------------------------------------------------------------------------
add_library(deps_3rdparty INTERFACE)

# 1. Handle Boost Linking
# Logic update: Check for targets because macOS might not find Boost::system (header-only)
if(TARGET Boost::system)
  # If system component is found (Linux or older Boost), link it
  target_link_libraries(deps_3rdparty INTERFACE Boost::system)
elseif(TARGET Boost::headers)
  # Standard target for header-only Boost in CMake built-in FindBoost
  target_link_libraries(deps_3rdparty INTERFACE Boost::headers)
elseif(TARGET Boost::boost)
  # Fallback target for Boost Config mode or some versions
  target_link_libraries(deps_3rdparty INTERFACE Boost::boost)
else()
  # Extreme case: No target found, fallback to legacy variables
  target_include_directories(deps_3rdparty INTERFACE ${Boost_INCLUDE_DIRS})
  target_link_libraries(deps_3rdparty INTERFACE ${Boost_LIBRARIES})
endif()

# If Windows, link socket libraries
if(WIN32)
  target_link_libraries(deps_3rdparty INTERFACE wsock32 ws2_32)
endif()

# 2. Handle fmt
# Use fmt::fmt target to ensure correct include/library paths are used
# This fixes "ld: library 'fmt' not found" on macOS
target_link_libraries(deps_3rdparty INTERFACE fmt::fmt)

# 3. Handle other Generic Packages (Convert variables to Interface properties)
foreach(package ${packages})
  # Boost and fmt are already handled separately, skip them here
  if(NOT "${package}" STREQUAL "Boost" AND NOT "${package}" STREQUAL "fmt")
    link_legacy_package_vars(deps_3rdparty ${package})
    message(STATUS "Processed dependencies for: ${package}")
  endif()
endforeach()

# ------------------------------------------------------------------------------
# Targets
# ------------------------------------------------------------------------------

# --- Internal API Library ---
# Logic from sweet_config(api ...) and sweet_target(api ...) integrated here
add_library(api STATIC
  src/api/api.cpp
  src/api/dispatcher.cpp
  src/api/Detect.cpp
)

# Link all 3rd-party dependencies
# Use PUBLIC because api headers might use types from these libs,
# and original sweet config implied propagation to consumers (internal-api)
target_link_libraries(api PUBLIC deps_3rdparty)

# Set Include Paths (originally sweet_config(api INCLUDES ...))
# Use PUBLIC so executables linking api automatically get this include path
target_include_directories(api PUBLIC "${PROJECT_SOURCE_DIR}/include")


# --- Executables ---
# add_executable(hotfix-server  src/hotfix-server/main.cpp)
# add_executable(hotfix-client  src/hotfix-client/main.cpp)
# add_executable(simple-server  src/simple-server/main.cpp)
# add_executable(simple-client  src/simple-client/main.cpp)
add_executable(server       src/server/main.cpp)
add_executable(hint-client  src/hint-client/main.cpp)
add_executable(image-client src/image-client/main.cpp)
add_executable(audio-client src/audio-client/main.cpp)

# Set Executable Linking
# Originally sweet_target(xxx CONFIGS ${packages} internal-api)
# Since 'api' already PUBLIC links 'deps_3rdparty', linking 'api' here is sufficient.
# But listing deps_3rdparty explicitly is also fine for safety.

foreach(tgt server hint-client image-client audio-client)
  target_link_libraries(${tgt} PRIVATE api deps_3rdparty)
endforeach()

# ------------------------------------------------------------------------------
# Post-build Actions
# ------------------------------------------------------------------------------
configure_file(src/server/database_decr.dat "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/database_decr.dat" COPYONLY)
configure_file(src/server/database_name.dat "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/database_name.dat" COPYONLY)
configure_file(src/server/shape_predictor_68_face_landmarks.dat "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shape_predictor_68_face_landmarks.dat" COPYONLY)
configure_file(src/server/Training_official.dat "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Training_official.dat" COPYONLY)