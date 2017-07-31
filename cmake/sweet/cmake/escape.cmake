## cmake/escape.cmake --- escape semicolons in value
##
## Copyright (c) 2017 ChienYu Lin
##
## Author: ChienYu Lin <cy20lin@gmail.com>
## License: MIT
##

function(sweet_escape_value output value)
  string(REPLACE "\\" "\\\\" ${output} "${value}")
  string(REPLACE ";" "\\;" ${output} "${value}")
  set(${output} "${${output}}" PARENT_SCOPE)
endfunction()

function(sweet_escape_values output)
  set("${output}" "")
  if(size EQUAL 1)
    return()
  endif()
  math(EXPR end "${ARGC} - 1")
  foreach(i RANGE 1 ${end})
    sweet_escape_value(value "${ARGV${i}}")
    list(APPEND "${output}" "${value}")
  endforeach()
  set("${output}" "${${output}}" PARENT_SCOPE)
endfunction()
