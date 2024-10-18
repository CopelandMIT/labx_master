macro(debugMsg msg)
    message(STATUS "DEBUG: ${msg}")
endmacro()

macro(debugVar var)
    debugMsg("${var}=\${${var}}")
endmacro()


if(NOT CMAKE_PROPERTY_LIST)
    execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)

    STRING(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
    STRING(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")

    list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)
endif()

function(print_target_properties target)
    message ("properties for target: ${target}")
    foreach (property ${CMAKE_PROPERTY_LIST})
        if(NOT ${property} MATCHES LOCATION)
            string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" property ${property})
            get_target_property(value ${target} ${property})
            if (value)
                message ("    ${property}  =  ${value}")
            endif()
        endif()
    endforeach()
endfunction()
