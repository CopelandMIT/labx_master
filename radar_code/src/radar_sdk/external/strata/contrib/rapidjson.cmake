set(RAPIDJSON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include")

file(GLOB_RECURSE RAPIDJSON_FILES ${RAPIDJSON_DIR}/*.h)

add_interface_library(rapidjson ${RAPIDJSON_FILES})
interface_target_include_directories(rapidjson ${RAPIDJSON_DIR})
interface_target_compile_definitions(rapidjson RAPIDJSON_HAS_STDSTRING)
