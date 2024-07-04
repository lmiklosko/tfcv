
# Extract the directory where *this* file has been installed (determined at cmake run-time)
# Get the absolute path with no ../.. relative marks, to eliminate implicit linker warnings
get_filename_component(@CMAKE_PROJECT_NAME@_CONFIG_PATH "${CMAKE_CURRENT_LIST_DIR}" REALPATH)
get_filename_component(@CMAKE_PROJECT_NAME@_INSTALL_PATH "${@CMAKE_PROJECT_NAME@_CONFIG_PATH}/../../../" REALPATH)

add_library(@CMAKE_PROJECT_NAME@ SHARED IMPORTED)
set_target_properties(@CMAKE_PROJECT_NAME@ PROPERTIES
  IMPORTED_LOCATION "${@CMAKE_PROJECT_NAME@_INSTALL_PATH}/lib@CMAKE_PROJECT_NAME@.so"
  INTERFACE_INCLUDE_DIRECTORIES "${@CMAKE_PROJECT_NAME@_INSTALL_PATH}/include/@CMAKE_PROJECT_NAME@"
)