add_library(dta_loop_functions MODULE
   dta_loop_functions.h
   dta_loop_functions.cpp)

target_link_libraries(dta_loop_functions
   ${SROCS_ENTITIES_LIBRARY})

