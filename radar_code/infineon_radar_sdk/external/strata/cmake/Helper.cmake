
function(add_subdirectory_optional dir)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/CMakeLists.txt")
        add_subdirectory(${dir} ${ARGN})
    endif()
endfunction()


function(get_external_link_libraries target RESULT)
    set(EXTERNAL_LIBS "")
    get_target_property(LINK_LIBRARIES ${target} INTERFACE_LINK_LIBRARIES)
    if(NOT LINK_LIBRARIES STREQUAL LINK_LIBRARIES-NOTFOUND)
        foreach(lib ${LINK_LIBRARIES})
            if (NOT TARGET ${lib} AND NOT "${lib}" MATCHES "^\\$\\<LINK_ONLY:")
                list(APPEND EXTERNAL_LIBS ${lib})
            endif()
        endforeach()
    endif()

    list(REMOVE_DUPLICATES EXTERNAL_LIBS)
    set(${RESULT} ${EXTERNAL_LIBS} PARENT_SCOPE)
endfunction()


# from cmake 3.19 onwards, we can finally do the following
# and later simply use target_link_libraries()
# (but we still don't for backwards compatibility)
#
#add_library(${name} INTERFACE ${ARGN})
#target_include_directories(${name} INTERFACE ${ARGN})
#target_compile_definitions(${name} INTERFACE ${ARGN})

function(add_interface_library name)
    add_custom_target(${name} SOURCES ${ARGN})
endfunction()

function(interface_target_include_directories name)
    set_target_properties(${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${ARGN})
endfunction()

function(interface_target_compile_definitions name)
    set_target_properties(${name} PROPERTIES INTERFACE_COMPILE_DEFINITIONS ${ARGN})
endfunction()


# Linking to object libraries and to our workaround interface libraries has to be implemented manually:

function(target_link_libraries_helper target)
    # determine the link type for object dependencies
    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL OBJECT_LIBRARY)
        set(LINK_TYPE PUBLIC)
    else()
        set(LINK_TYPE PRIVATE)
    endif()

    foreach(item ${ARGN})
        # populating the interface properties is the same for both interface and object libraries

        if(NOT TARGET ${item})
            target_link_libraries(${target} ${LINK_TYPE} ${item})
        else()
            get_target_property(item_type ${item} TYPE)
            if(item_type STREQUAL OBJECT_LIBRARY)
                set(PROPERTY_PREFIX "")
            else()
                set(PROPERTY_PREFIX "INTERFACE_")
            endif()

            get_target_property(INCLUDE_DIRS ${item} ${PROPERTY_PREFIX}INCLUDE_DIRECTORIES)
            if(NOT INCLUDE_DIRS STREQUAL INCLUDE_DIRS-NOTFOUND)
                list(APPEND TARGET_INCLUDE_DIRS ${INCLUDE_DIRS})
            endif()

            get_target_property(COMPILE_DEFS ${item} ${PROPERTY_PREFIX}COMPILE_DEFINITIONS)
            if(NOT COMPILE_DEFS STREQUAL COMPILE_DEFS-NOTFOUND)
                list(APPEND TARGET_COMPILE_DEFS ${COMPILE_DEFS})
            endif()

            get_target_property(LINK_LIBRARIES ${item} ${PROPERTY_PREFIX}LINK_LIBRARIES)
            if(NOT LINK_LIBRARIES STREQUAL LINK_LIBRARIES-NOTFOUND)
                list(APPEND TARGET_LINK_LIBRARIES ${LINK_LIBRARIES})
            endif()

            get_target_property(LINK_DIRECTORIES ${item} ${PROPERTY_PREFIX}LINK_DIRECTORIES)
            if(NOT LINK_DIRECTORIES STREQUAL LINK_DIRECTORIES-NOTFOUND)
                list(APPEND TARGET_LINK_DIRECTORIES ${LINK_DIRECTORIES})
            endif()

            # if we have an object dependency, we need to handle its source elements
            get_target_property(type ${item} TYPE)
            if(type STREQUAL OBJECT_LIBRARY)
                get_target_property(SOURCES ${item} SOURCES)
                foreach(source ${SOURCES})
                    if ("${source}" MATCHES "^\\$\\<TARGET_OBJECTS:")
                        target_sources(${target} ${LINK_TYPE} ${source})
                    endif()
                endforeach()

                target_sources(${target} ${LINK_TYPE} $<TARGET_OBJECTS:${item}>)
            endif()
        endif()
    endforeach()

    if(TARGET_INCLUDE_DIRS)
        list(REMOVE_DUPLICATES TARGET_INCLUDE_DIRS)
        target_include_directories(${target} PUBLIC ${TARGET_INCLUDE_DIRS})
    endif()
    if(TARGET_COMPILE_DEFS)
        list(REMOVE_DUPLICATES TARGET_COMPILE_DEFS)
        target_compile_definitions(${target} PUBLIC ${TARGET_COMPILE_DEFS})
    endif()
    if(TARGET_LINK_LIBRARIES)
        list(REMOVE_DUPLICATES TARGET_LINK_LIBRARIES)
        target_link_libraries(${target} PUBLIC ${TARGET_LINK_LIBRARIES})
    endif()
    if(TARGET_LINK_DIRECTORIES)
        list(REMOVE_DUPLICATES TARGET_LINK_DIRECTORIES)
        target_link_directories(${target} PUBLIC ${TARGET_LINK_DIRECTORIES})
    endif()

endfunction()
