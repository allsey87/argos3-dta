# find the include directory
find_path (ARGOS_INCLUDE_DIR argos3/core/config.h)

if(NOT ARGOS_INCLUDE_DIR)
   message(FATAL_ERROR "Can not locate the header file: argos3/core/config.h")
endif(NOT ARGOS_INCLUDE_DIR)

# read the config.h file to get the installations configuration
file(READ ${ARGOS_INCLUDE_DIR}/argos3/core/config.h ARGOS_CONFIGURATION)

# parse the build_for variable
string(REGEX MATCH "\#define ARGOS_BUILD_FOR \"([^\"]*)\"" UNUSED_VARIABLE ${ARGOS_CONFIGURATION})
set(ARGOS_BUILD_FOR ${CMAKE_MATCH_1})
if(ARGOS_BUILD_FOR STREQUAL "simulator")
  set(ARGOS_BUILD_FOR_SIMULATOR TRUE)
else(ARGOS_BUILD_FOR STREQUAL "simulator")
  set(ARGOS_BUILD_FOR_SIMULATOR FALSE)
endif(ARGOS_BUILD_FOR STREQUAL "simulator")

# check and set ARGOS_USE_DOUBLE
if(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_USE_DOUBLE")
  set(ARGOS_USE_DOUBLE ON)
else(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_USE_DOUBLE")
  set(ARGOS_USE_DOUBLE OFF)
endif(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_USE_DOUBLE")

# check and set ARGOS_WITH_LUA
if(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_WITH_LUA")
  set(ARGOS_WITH_LUA ON)
else(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_WITH_LUA")
  set(ARGOS_WITH_LUA OFF)
endif(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_WITH_LUA")

# check and set ARGOS_COMPILE_QTOPENGL
if(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_COMPILE_QTOPENGL")
  set(ARGOS_COMPILE_QTOPENGL ON)
else(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_COMPILE_QTOPENGL")
  set(ARGOS_COMPILE_QTOPENGL OFF)
endif(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_COMPILE_QTOPENGL")

# check and set ARGOS_WITH_GSL
if(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_WITH_GSL")
  set(ARGOS_WITH_GSL ON)
else(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_WITH_GSL")
  set(ARGOS_WITH_GSL OFF)
endif(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_WITH_GSL")

# check and set ARGOS_WITH_FREEIMAGE
if(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_WITH_FREEIMAGE")
  set(ARGOS_WITH_FREEIMAGE ON)
else(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_WITH_FREEIMAGE")
  set(ARGOS_WITH_FREEIMAGE OFF)
endif(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_WITH_FREEIMAGE")

# check and set ARGOS_THREADSAFE_LOG
if(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_THREADSAFE_LOG")
  set(ARGOS_THREADSAFE_LOG ON)
else(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_THREADSAFE_LOG")
  set(ARGOS_THREADSAFE_LOG OFF)
endif(${ARGOS_CONFIGURATION} MATCHES "\#define ARGOS_THREADSAFE_LOG")
