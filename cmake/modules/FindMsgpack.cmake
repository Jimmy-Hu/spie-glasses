##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
##

# - Try to find Msgpack
# Once done this will define
#  Msgpack_FOUND - System has Msgpack
#  Msgpack_INCLUDE_DIRS - The Msgpack include directories
#  Msgpack_LIBRARIES - The libraries needed to use Msgpack
#  Msgpack_DEFINITIONS - Compiler switches required for using Msgpack


find_path(Msgpack_INCLUDES NAMES msgpack.h msgpack.hpp PATH_SUFFIXES msgpack)
if (Msgpack_STATIC)
  find_library(Msgpack_LIBRARIES NAMES libmsgpackc.a msgpackc libmsgpackc )
else()
  find_library(Msgpack_LIBRARIES NAMES msgpackc libmsgpackc)
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set Msgpack_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Msgpack DEFAULT_MSG
                                  Msgpack_LIBRARIES Msgpack_INCLUDES)

set(Msgpack_LIBRARY      ${Msgpack_LIBRARIES})
set(Msgpack_LIBS         ${Msgpack_LIBRARIES})
set(Msgpack_INCLUDE_DIRS ${Msgpack_INCLUDES})
set(Msgpack_INCLUDE_DIR  ${Msgpack_INCLUDES})
# set(Msgpack_DEFINITIONS  "")
