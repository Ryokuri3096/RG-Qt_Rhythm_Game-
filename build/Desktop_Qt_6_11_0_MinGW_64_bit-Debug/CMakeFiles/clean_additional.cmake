# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\RG_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\RG_autogen.dir\\ParseCache.txt"
  "RG_autogen"
  )
endif()
