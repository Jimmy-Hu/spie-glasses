##
## CMakeLists.txt (Rewritten to standard CMake)
##
## Original Author: ChienYu Lin
##
cmake_minimum_required (VERSION 3.5)											#	set cmake minimum version

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

macro(link_legacy_package_vars target_name package_name)
  if(${package_name}_FOUND OR ${package_name})
    foreach(suffix LIBRARIES LIBRARY LIBS)
      if (DEFINED ${package_name}_${suffix} AND NOT "${${package_name}_${suffix}}" STREQUAL "")
        target_link_libraries(${target_name} INTERFACE ${${package_name}_${suffix}})
        break()
      endif()
    endforeach()

    foreach(suffix INCLUDES INCLUDE_DIR INCLUDE_DIRS)
      if (DEFINED ${package_name}_${suffix} AND NOT "${${package_name}_${suffix}}" STREQUAL "")
        target_include_directories(${target_name} INTERFACE ${${package_name}_${suffix}})
        break()
      endif()
    endforeach()

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

# [Platform Logic]
if(APPLE)
  set(Boost_USE_STATIC_LIBS OFF)
else()
  set(Boost_USE_STATIC_LIBS ON)
endif()

set(Boost_COMPONENTS system)

find_package(Boost REQUIRED COMPONENTS ${Boost_COMPONENTS})
find_package(fmt CONFIG REQUIRED)
set(Msgpack_STATIC ON)
find_package(Msgpack REQUIRED)
set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED)
find_package(Json REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LZ4 REQUIRED)
find_package(dlib REQUIRED)

set(packages Boost fmt Msgpack OpenCV ZLIB dlib Json LZ4)

add_library(deps_3rdparty INTERFACE)

foreach(component ${Boost_COMPONENTS})
  if(TARGET Boost::${component})
    target_link_libraries(deps_3rdparty INTERFACE Boost::${component})
  endif()
endforeach()

if(WIN32)
  target_link_libraries(deps_3rdparty INTERFACE wsock32 ws2_32)
endif()

target_link_libraries(deps_3rdparty INTERFACE fmt)

foreach(package ${packages})
  link_legacy_package_vars(deps_3rdparty ${package})
  message(STATUS "Processed dependencies for: ${package}")
endforeach()

# ------------------------------------------------------------------------------
# Targets
# ------------------------------------------------------------------------------

# --- Internal API Library ---
add_library(api STATIC
  src/api/api.cpp
  src/api/dispatcher.cpp
  src/api/Detect.cpp
)

target_link_libraries(api PUBLIC deps_3rdparty)

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