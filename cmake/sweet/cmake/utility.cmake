## cmake/utility.cmake --- some other utilities
##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
## License: MIT
##

macro(set_if_not_defined var)
  if (NOT DEFINED ${var})
    set(${var} ${ARGN})
  endif()
endmacro()

function(is_begin_with var value)
  set(${var} 0 PARENT_SCOPE)
  foreach(prefix IN LISTS ARGN)
    string(LENGTH "${value}" value_len)
    string(LENGTH "${prefix}" prefix_len)
    if (prefix_len GREATER value_len)
      continue()
    endif()
    string(SUBSTRING "${value}" 0 ${prefix_len} substr)
    if (substr STREQUAL prefix)
      set(${var} 1 PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

function(is_end_with var value)
  set(${var} 0 PARENT_SCOPE)
  foreach(suffix IN LISTS ARGN)
    string(LENGTH "${value}" value_len)
    string(LENGTH "${suffix}" suffix_len)
    if (suffix_len GREATER value_len)
      continue()
    endif()
    math(EXPR begin "${value_len} - ${suffix_len}")
    string(SUBSTRING "${value}" ${begin} ${suffix_len} substr)
    if (substr STREQUAL suffix)
      set(${var} 1 PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

function(keep_if_begin_with out_list in_list)
  foreach(value IN LISTS in_list)
    is_begin_with(_ "${value}" ${ARGN})
    if (_)
      list(APPEND result "${value}")
    endif()
  endforeach()
  set(${out_list} "${result}" PARENT_SCOPE)
endfunction()

function(keep_if_end_with out_list in_list)
  foreach(value IN LISTS in_list)
    is_end_with(_ "${value}" ${ARGN})
    if (_)
      list(APPEND result "${value}")
    endif()
  endforeach()
  set(${out_list} "${result}" PARENT_SCOPE)
endfunction()

function(keep_if_matches)
endfunction()

function(remove_if_begin_with out_list in_list)
  foreach(value IN LISTS in_list)
    is_begin_with(_ "${value}" ${ARGN})
    if (NOT _)
      list(APPEND result "${value}")
    endif()
  endforeach()
  set(${out_list} "${result}" PARENT_SCOPE)
endfunction()

function(remove_if_end_with out_list in_list)
  foreach(value IN LISTS in_list)
    is_end_with(_ "${value}" ${ARGN})
    if (NOT _)
      list(APPEND result "${value}")
    endif()
  endforeach()
  set(${out_list} "${result}" PARENT_SCOPE)
endfunction()

function(remove_if_matches)
endfunction()

function(remove_if_is_directory out_list in_list)
  foreach(value IN LISTS in_list)
    if (NOT IS_DIRECTORY "${value}")
      list(APPEND result "${value}")
    endif()
  endforeach()
  set(${out_list} "${result}" PARENT_SCOPE)
endfunction()
