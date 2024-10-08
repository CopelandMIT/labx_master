
macro(add_subdirectories)
    file(GLOB dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
    foreach(dir ${dirs})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/CMakeLists.txt")
            add_subdirectory(${dir} ${ARGN})
        endif()
    endforeach()
endmacro()
