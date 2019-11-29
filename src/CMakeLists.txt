#
# Set minimum required version
#
cmake_minimum_required(VERSION 3.1)

project(argos3-dta CXX)

#
# Set the path additional cmake files must be searched for
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

#
# Set build options, set flags, check libraries
#
include(${CMAKE_SOURCE_DIR}/cmake/ARGoSBuildOptions.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/ARGoSBuildFlags.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/ARGoSBuildChecks.cmake)

# Set up ARGoS compilation information
include_directories(BEFORE ${CMAKE_SOURCE_DIR})

#
# Compile loop function
#
add_subdirectory(loop_functions)

#
# Configure examples
#
configure_file(
  ${CMAKE_SOURCE_DIR}/experiment/luabt.lua
  ${CMAKE_BINARY_DIR}/experiment/luabt.lua
  COPYONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/experiment/abstract.argos.in
  ${CMAKE_BINARY_DIR}/experiment/abstract.argos)

configure_file(
  ${CMAKE_SOURCE_DIR}/experiment/pipuck.lua
  ${CMAKE_BINARY_DIR}/experiment/pipuck.lua
  COPYONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/experiment/srocs.argos.in
  ${CMAKE_BINARY_DIR}/experiment/srocs.argos)

configure_file(
  ${CMAKE_SOURCE_DIR}/experiment/builderbot.lua
  ${CMAKE_BINARY_DIR}/experiment/builderbot.lua
  COPYONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/experiment/block.lua
  ${CMAKE_BINARY_DIR}/experiment/block.lua
  COPYONLY)
