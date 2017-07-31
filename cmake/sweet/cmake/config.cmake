## cmake/config.cmake --- config manipulating utilities
##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
## License: MIT
##

## public interfaces ##

function(sweet_config_add config type)
  if(type STREQUAL "CONFIGURE")
    __sweet_config_add_configure("${config}" ${ARGN})
  else()
    __sweet_config_add_basic("${config}" "${type}" ${ARGN})
  endif()
  set("${config}_CONFIG_SETTINGS" "${${config}_CONFIG_SETTINGS}" PARENT_SCOPE)
endfunction()

function(sweet_config_reset config)
  set("${config}_CONFIG_SETTINGS" "" PARENT_SCOPE)
endfunction()

function(sweet_config_unset config)
  unset("${config}_CONFIG_SETTINGS" PARENT_SCOPE)
endfunction()

## implementation details ##

function(__sweet_config_add_basic config)
  sweet_escape_value(ARGN "${ARGN}")
  list(APPEND "${config}_CONFIG_SETTINGS" "${ARGN}")
  set("${config}_CONFIG_SETTINGS" "${${config}_CONFIG_SETTINGS}" PARENT_SCOPE)
endfunction()

function(__sweet_config_add_configure config)
  string(CONFIGURE "${ARGN}" ARGN)
  __sweet_config_add_basic("${config}" ${ARGN})
  set("${config}_CONFIG_SETTINGS" "${${config}_CONFIG_SETTINGS}" PARENT_SCOPE)
endfunction()

function(__sweet_config_dump config)
  foreach(args IN LISTS ${config}_CONFIG_SETTINGS)
    message("target_add(${config};${args})")
  endforeach()
endfunction()

function(__sweet_config_insert config index)
  sweet_escape_value(ARGN "${ARGN}")
  list(INSERT "${config}_CONFIG_SETTINGS" "${index}" "${ARGN}")
  set("${config}_CONFIG_SETTINGS" "${${config}_CONFIG_SETTINGS}" PARENT_SCOPE)
endfunction()

macro(__sweet_config_prepend config)
  __sweet_config_insert("${config}" 0 "${ARGN}")
endmacro()

function(__sweet_config_append config) 
  sweet_escape_value(ARGN "${ARGN}")
  list(APPEND "${config}_CONFIG_SETTINGS" "${ARGN}")
  set("${config}_CONFIG_SETTINGS" "${${config}_CONFIG_SETTINGS}" PARENT_SCOPE)
endfunction()
