function(collect_dependencies input_target)
    get_target_property(public_dependencies ${input_target} LINK_LIBRARIES)
    get_target_property(interface_dependencies ${input_target} INTERFACE_LINK_LIBRARIES)
    list(APPEND public_dependencies ${interface_dependencies})
    foreach(dependency IN LISTS public_dependencies)
        if(TARGET ${dependency})
            get_target_property(alias ${dependency} ALIASED_TARGET)
            if (TARGET ${alias})
                set(dependency ${alias})
            endif()

            get_target_property(_type ${dependency} TYPE)
            if (${_type} STREQUAL "STATIC_LIBRARY")
                list(APPEND static_libs ${dependency})
            endif()

            get_property(library_already_added
                    GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency})
            if (NOT library_already_added)
                set_property(GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency} ON)
                collect_dependencies(${dependency})
                list(APPEND static_libs ${${dependency}_DEPENDENCIES})
            endif()
        endif()
    endforeach()

    list(REMOVE_DUPLICATES static_libs)
    set(${input_target}_DEPENDENCIES "${static_libs}" PARENT_SCOPE)
endfunction()