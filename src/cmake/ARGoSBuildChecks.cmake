#
# Check for correct GCC version
#
macro(CHECK_GCC MIN_VERSION)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  if(GCC_VERSION VERSION_GREATER ${MIN_VERSION} OR GCC_VERSION VERSION_EQUAL ${MIN_VERSION})
    message(STATUS "GCC/G++ version >= ${MIN_VERSION}")
  else(GCC_VERSION VERSION_GREATER ${MIN_VERSION} OR GCC_VERSION VERSION_EQUAL ${MIN_VERSION})
    message(FATAL_ERROR "You need to have at least GCC/G++ version ${MIN_VERSION}!")
  endif(GCC_VERSION VERSION_GREATER ${MIN_VERSION} OR GCC_VERSION VERSION_EQUAL ${MIN_VERSION})
endmacro(CHECK_GCC)

#
# Set variables depending on current compiler
#
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  set(ARGOS_START_LIB_GROUP)
  set(ARGOS_END_LIB_GROUP)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(ARGOS_START_LIB_GROUP -Wl,--start-group)
  set(ARGOS_END_LIB_GROUP -Wl,--end-group)
  check_gcc(6.1.0)
endif()

#
# Check for Lua 5.3
#
find_package(Lua53)
if(LUA53_FOUND)
  set(ARGOS_WITH_LUA ON)
  include_directories(${LUA_INCLUDE_DIR})
else(LUA53_FOUND)
  message(FATAL_ERROR "Lua 5.3 not found")
endif(LUA53_FOUND)

#
# Check if ARGoS-SRoCS is installed
#
find_package(SRoCS)

if(NOT SROCS_FOUND)
  message(FATAL_ERROR "The SRoCS plugins for ARGoS can not be found.")
endif(NOT SROCS_FOUND)
include_directories(${ARGOS_INCLUDE_DIR})

