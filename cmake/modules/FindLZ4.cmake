##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
##

# - Try to find LZ4
# Once done this will define
#  LZ4_FOUND - System has LZ4
#  LZ4_INCLUDE_DIRS - The LZ4 include directories
#  LZ4_LIBRARIES - The libraries needed to use LZ4
#  LZ4_DEFINITIONS - Compiler switches required for using LZ4


find_path(LZ4_INCLUDES NAMES lz4.h)
if (LZ4_STATIC)
  find_library(LZ4_LIBRARIES NAMES liblz4.a lz4 liblz4)
else()
  find_library(LZ4_LIBRARIES NAMES lz4 liblz4)
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LZ4_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LZ4 DEFAULT_MSG
                                  LZ4_LIBRARIES LZ4_INCLUDES)

set(LZ4_LIBRARY      ${LZ4_LIBRARIES})
set(LZ4_LIBS         ${LZ4_LIBRARIES})
set(LZ4_INCLUDE_DIRS ${LZ4_INCLUDES})
set(LZ4_INCLUDE_DIR  ${LZ4_INCLUDES})
# set(LZ4_DEFINITIONS  "")
