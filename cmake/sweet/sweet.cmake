cmake_minimum_required(VERSION 3.1)

get_property(__is_sweet_included GLOBAL PROPERTY SWEET_INCLUDED)
if(__is_sweet_included)
  return()
endif()
message(STATUS "include sweet internal cmake files")
set_property(GLOBAL PROPERTY SWEET_INCLUDED 1)
include("${CMAKE_CURRENT_LIST_DIR}/cmake/all.cmake")
