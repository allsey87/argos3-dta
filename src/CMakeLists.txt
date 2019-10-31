#
# Set minimum required version
#
cmake_minimum_required(VERSION 3.1)

project(argos3-dta-loopfunction)

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
add_subdirectory(loop-function)

#
# Create .gdbinit file for GDB support
# It is placed in the same directory as the argos3 executable
#
configure_file(
  ${CMAKE_SOURCE_DIR}/scripts/gdbinit.in
  ${CMAKE_BINARY_DIR}/core/.gdbinit
  @ONLY)

#
# Create setup_envsh file for bash support
# It is placed in the base build directory
#
configure_file(
  ${CMAKE_SOURCE_DIR}/scripts/setup_env.sh.in
  ${CMAKE_BINARY_DIR}/setup_env.sh
  @ONLY)

