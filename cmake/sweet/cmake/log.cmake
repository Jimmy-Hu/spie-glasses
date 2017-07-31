## cmake/log.cmake --- loggging utilities
##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
## License: MIT
##

macro(log_variables)
  foreach(var ${ARGN})
    message("${var}=${${var}}")
  endforeach()
endmacro()

