include(${CMAKE_CURRENT_LIST_DIR}/Platform.cmake)

set(STRATA_LIB_DIR "${STRATA_BIN_DIR}/${STRATA_TARGET_PLATFORM}/x${STRATA_BITNESS}")
set(STRATA_LIB_SUFFIX $<$<CONFIG:Debug>:-d>)


set(DEP_FILE "${STRATA_LIB_DIR}/deps.txt")
if(EXISTS "${DEP_FILE}")
    file(READ "${DEP_FILE}" STRATA_LIBRARY_DEPENDENCIES)
else()
    message(WARNING "Dependency information not found, link libraries have to be specified manually or linking might fail.")
endif()
