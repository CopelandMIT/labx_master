
set(MODULES_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IModule.hpp"
    )

set(MODULES_SOURCES
    )

add_library(modules OBJECT ${MODULES_HEADERS} ${MODULES_SOURCES})
set_target_properties(modules PROPERTIES LINKER_LANGUAGE CXX) # remove this line once .cpp files have been added
