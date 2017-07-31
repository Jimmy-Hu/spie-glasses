##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
##

# - Try to find Json
# Once done this will define
#  Json_FOUND - System has Json
#  Json_INCLUDE_DIRS - The Json include directories
#  Json_LIBRARIES - The libraries needed to use Json
#  Json_DEFINITIONS - Compiler switches required for using Json


find_path(Json_INCLUDES json.hpp nlohmann/json.hpp)
# find_library(Json_LIBRARIES json)
set(Json_LIBRARIES "")


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set Json_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Json DEFAULT_MSG Json_INCLUDES)

set(Json_LIBRARY      ${Json_LIBRARIES})
set(Json_LIBS         ${Json_LIBRARIES})
set(Json_INCLUDE_DIRS ${Json_INCLUDES})
set(Json_INCLUDE_DIR  ${Json_INCLUDES})
