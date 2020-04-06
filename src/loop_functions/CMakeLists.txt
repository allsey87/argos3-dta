add_library(dta_abstract_loop_functions MODULE
   dta_abstract_loop_functions.h
   dta_abstract_loop_functions.cpp)

target_link_libraries(dta_abstract_loop_functions
   ${SROCS_ENTITIES_LIBRARY})

