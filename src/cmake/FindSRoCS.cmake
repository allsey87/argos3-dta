#=============================================================================
# - Find the SRoCS Library
#=============================================================================
# This module defines
# 	SROCS_INCLUDE_DIR, where to find argos3/core/config.h, etc.
# 	SROCS_ENTITIES_LIBRARY, the SRoCS entity library
# 	SROCS_FOUND, true if SRoCS was found.
#
#=============================================================================

set(SROCS_FOUND 0)

# find the include directory
find_path (ARGOS_INCLUDE_DIR argos3/core/config.h)

if(NOT ARGOS_INCLUDE_DIR)
   message(FATAL_ERROR "Can not locate the header file: argos3/core/config.h")
endif(NOT ARGOS_INCLUDE_DIR)

# read the config.h file to get the installations configuration
file(READ ${ARGOS_INCLUDE_DIR}/argos3/core/config.h ARGOS_CONFIGURATION)

# parse and version and release strings
string(REGEX MATCH "\#define ARGOS_VERSION \"([^\"]*)\"" UNUSED_VARIABLE ${ARGOS_CONFIGURATION})
set(ARGOS_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "\#define ARGOS_RELEASE \"([^\"]*)\"" UNUSED_VARIABLE ${ARGOS_CONFIGURATION})
set(ARGOS_RELEASE ${CMAKE_MATCH_1})
set(ARGOS_VERSION_RELEASE "${ARGOS_VERSION}-${ARGOS_RELEASE}")

# find the core library
find_library(SROCS_ENTITIES_LIBRARY
  NAMES argos3srocs_${ARGOS_BUILD_FOR}_entities
  PATH_SUFFIXES argos3
  DOC "The SRoCS entity library"
)

#=============================================================================

INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (
  SRoCS
  FOUND_VAR SROCS_FOUND
  REQUIRED_VARS ARGOS_INCLUDE_DIR SROCS_ENTITIES_LIBRARY
  VERSION_VAR ARGOS_VERSION_RELEASE
#  HANDLE_COMPONENTS
)

#  
