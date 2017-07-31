##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
##

# - Try to find FPlus
# Once done this will define
#  FPlus_FOUND - System has Msgpack
#  FPlus_INCLUDE_DIRS - The Msgpack include directories
#  FPlus_LIBRARIES - The libraries needed to use Msgpack
#  FPlus_DEFINITIONS - Compiler switches required for using Msgpack


find_path(FPlus_INCLUDES fplus/fplus.hpp)
set(FPlus_LIBRARIES "")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FPlus_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(FPlus DEFAULT_MSG FPlus_INCLUDES)

set(FPlus_INCLUDE_DIRS ${Msgpack_INCLUDES})
set(FPlus_INCLUDE_DIR  ${Msgpack_INCLUDES})
