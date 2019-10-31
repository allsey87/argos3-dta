add_library(srocs_dta_loop_functions MODULE
   srocs_dta_loop_functions.h
   srocs_dta_loop_functions.cpp)

target_link_libraries(srocs_dta_loop_functions
   ${SROCS_ENTITIES_LIBRARY})

install(TARGETS srocs_dta_loop_functions
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib/argos3
   ARCHIVE DESTINATION lib/argos3)

