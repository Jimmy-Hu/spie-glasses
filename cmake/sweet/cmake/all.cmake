## cmake/all.cmake --- include all cmake files
##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
## License: MIT
##

set(
  __core_cmake_files
  utility.cmake
  escape.cmake
  )

message(STATUS "in directory '${CMAKE_CURRENT_LIST_DIR}'")

file(GLOB __files RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/*.cmake")
set(__non_core_cmake_files "${__files}")
list(REMOVE_ITEM __non_core_cmake_files ${__core_cmake_files} all.cmake)

message(STATUS "include core cmake files")
foreach(file IN LISTS __core_cmake_files)
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/${file}" AND
      NOT IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/${file}" AND
      NOT file MATCHES "^[.].*$")
    message(STATUS "  ${file}")
    include("${CMAKE_CURRENT_LIST_DIR}/${file}")
  endif()
endforeach()

message(STATUS "include other cmake files")
foreach(file IN LISTS __non_core_cmake_files)
  if (NOT IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/${file}" AND
      NOT file MATCHES "^[.].*$")
    # message(STATUS "  include(\"${file}\")")
    # message(STATUS "  \"${file}\"")
    message(STATUS "  ${file}")
    include("${CMAKE_CURRENT_LIST_DIR}/${file}")
  endif()
endforeach()
