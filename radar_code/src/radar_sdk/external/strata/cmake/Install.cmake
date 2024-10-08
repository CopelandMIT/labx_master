
function(install_target_helper target dir)
    get_target_property(type ${target} TYPE)
    if(type MATCHES MODULE_LIBRARY)
         set(ARTIFACT LIBRARY)
    else()
         set(ARTIFACT RUNTIME)
    endif()

    install(TARGETS ${target} ${ARTIFACT} DESTINATION ${dir})

    if(TARGET libusb_strata)
        install(TARGETS libusb_strata RUNTIME DESTINATION ${dir})
    endif()

    if(WIN32)
        set(ARTIFACT RUNTIME)
    else()
        set(ARTIFACT LIBRARY)
    endif()

    foreach(arg IN LISTS ARGN)
        install(TARGETS ${arg} ${ARTIFACT} DESTINATION ${dir})
    endforeach()
endfunction()
