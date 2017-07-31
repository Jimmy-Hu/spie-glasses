##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
##

# - Try to find YamlCpp
# Once done this will define
#  YamlCpp_FOUND - System has YamlCpp
#  YamlCpp_INCLUDE_DIRS - The YamlCpp include directories
#  YamlCpp_LIBRARIES - The libraries needed to use YamlCpp
#  YamlCpp_DEFINITIONS - Compiler switches required for using YamlCpp


find_path(YamlCpp_INCLUDES yaml-cpp/yaml.h)
find_library(YamlCpp_LIBRARIES yaml-cpp)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set YamlCpp_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(YamlCpp DEFAULT_MSG
                                  YamlCpp_LIBRARIES YamlCpp_INCLUDES)

set(YamlCpp_LIBRARY      ${YamlCpp_LIBRARIES})
set(YamlCpp_LIBS         ${YamlCpp_LIBRARIES})
set(YamlCpp_INCLUDE_DIRS ${YamlCpp_INCLUDES})
set(YamlCpp_INCLUDE_DIR  ${YamlCpp_INCLUDES})
