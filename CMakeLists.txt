##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
##
cmake_minimum_required (VERSION 3.1)

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
    # LOCAL # load "${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/config.cmake"
  )
endif()

include("cmake/sweet/sweet.cmake")

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

function(show_package_info package)
  message(STATUS "package=${package}")
  show_variables(
    ${package}_FOUND
    ${package}_LIBRARIES
    ${package}_LIBRARY
    ${package}_LIBS
    ${package}_INCLUDES
    ${package}_INCLUDE_DIR
    ${package}_INCLUDE_DIRS
    ${package}_DEFINITIONS
    )
endfunction()

macro(config_from_generic_package config package)
  if(${package})
    return()
  endif()
  foreach(suffix LIBRARIES LIBRARY LIBS)
    if (NOT "${${package}_${suffix}}" STREQUAL "")
      # [NOTE] CMP0023
      #
      # target_link_libraries(mylib A)
      # target_link_libraries(mylib PRIVATE B)
      #
      # The OLD behavior for this policy is to allow keyword and plain
      # target_link_libraries signatures to be mixed. The NEW behavior for
      # this policy is to not to allow mixing of the keyword and plain
      # signatures.
      #
      # reference https://github.com/haka-security/haka/issues/22
      sweet_config_add(${config} LIBRARIES PRIVATE ${${package}_${suffix}})
      break()
    endif()
  endforeach()
  foreach(suffix INCLUDES INCLUDE_DIR INCLUDE_DIRS)
    if (NOT "${${package}_${suffix}}" STREQUAL "")
      sweet_config_add(${config} INCLUDES PRIVATE ${${package}_${suffix}})
      break()
    endif()
  endforeach()
  foreach(suffix DEFINITIONS)
    if (NOT "${${package}_${suffix}}" STREQUAL "")
      sweet_config_add(${config} DEFINITIONS ${${package}_${suffix}})
      break()
    endif()
  endforeach()
endmacro()

# Q. can 'project(projname)' command used before 'include("cmake/HunterGate")' ??
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
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_COMPONENTS
  # atomic
  # chrono
  # container
  # context
  # coroutine
  # date_time
  # exception
  # fiber
  # filesystem
  # iostreams
  # locale
  # random
  # regex
  # serialization
  system
  # thread
  )

find_package(Boost REQUIRED COMPONENTS ${Boost_COMPONENTS})
find_package(fmt CONFIG REQUIRED)
set(Msgpack_STATIC ON)
find_package(Msgpack)
set(OpenCV_STATIC ON)
find_package(OpenCV)
find_package(Json)
find_package(ZLIB)
find_package(LZ4)
find_package(dlib)

# set(packages Boost fmt Msgpack Json FPlus OpenCV ZLIB)
set(packages Boost fmt Msgpack OpenCV ZLIB dlib Json LZ4)
foreach(component ${Boost_COMPONENTS} boost)
  if(TARGET Boost::${component})
    sweet_config_add(Boost LIBRARIES PRIVATE Boost::${component})
  endif()
endforeach()

if(WIN32)
  # boost.asio dependency
  sweet_config_add(Boost LIBRARIES PRIVATE wsock32 ws2_32)
endif()

sweet_config_add(fmt LIBRARIES PRIVATE fmt)

foreach(package ${packages})
  config_from_generic_package("${package}" "${package}")
  message(STATUS " config: ${package}=${${package}_CONFIG_SETTINGS}")
endforeach()

sweet_config_add(internal-api CONFIGS api)
sweet_config_add(api LIBRARIES PRIVATE api)
sweet_config_add(api INCLUDES  PRIVATE "${PROJECT_SOURCE_DIR}/include")
add_library(api STATIC
  src/api/api.cpp
  src/api/dispatcher.cpp
  src/api/Detect.cpp
  )

# add_executable(hotfix-server  src/hotfix-server/main.cpp)
# add_executable(hotfix-client  src/hotfix-client/main.cpp)
# add_executable(simple-server  src/simple-server/main.cpp)
# add_executable(simple-client  src/simple-client/main.cpp)
add_executable(server       src/server/main.cpp)
add_executable(hint-client  src/hint-client/main.cpp)
add_executable(image-client src/image-client/main.cpp)
add_executable(audio-client src/audio-client/main.cpp)

sweet_target_add(api          CONFIGS ${packages})
# sweet_target_add(hotfix-server  CONFIGS ${packages} internal-api)
# sweet_target_add(hotfix-client  CONFIGS ${packages} internal-api)
# sweet_target_add(old-server     CONFIGS ${packages} internal-api)
# sweet_target_add(old-client     CONFIGS ${packages} internal-api)
# sweet_target_add(simple-server  CONFIGS ${packages} internal-api)
# sweet_target_add(simple-client  CONFIGS ${packages} internal-api)
sweet_target_add(server       CONFIGS ${packages} internal-api)
sweet_target_add(hint-client  CONFIGS ${packages} internal-api)
sweet_target_add(image-client CONFIGS ${packages} internal-api)
sweet_target_add(audio-client CONFIGS ${packages} internal-api)
configure_file(src/server/database_decr.dat "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/database_decr.dat" COPYONLY)
configure_file(src/server/database_name.dat "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/database_name.dat" COPYONLY)
configure_file(src/server/shape_predictor_68_face_landmarks.dat "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shape_predictor_68_face_landmarks.dat" COPYONLY)
configure_file(src/server/Training_official.dat "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Training_official.dat" COPYONLY)

