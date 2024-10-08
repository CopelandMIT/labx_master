set(BUILD_SHARED_LIBS OFF)
add_subdirectory(pugixml EXCLUDE_FROM_ALL)
unset(BUILD_SHARED_LIBS)

target_sources(pugixml-static PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src/pugixml.hpp")
