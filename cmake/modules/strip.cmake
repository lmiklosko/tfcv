macro(target_strip_symbols target)
    if (NOT IOS AND target_type STREQUAL "SHARED_LIBRARY")
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${target}>
                COMMENT "Stripping symbols from ${target}"
        )
    endif ()
endmacro (target_strip_symbols)
