cmake_minimum_required(VERSION 3.23)
cmake_policy(SET CMP0065 NEW)

# Project settings
# ====================================================
project (tfcv
        VERSION 0.1.0
        DESCRIPTION "Pre-compiled tensorflow lite for computer vision"
        HOMEPAGE_URL "https://github.com/lmiklosko/tfcv"
)

option (BUILD_TESTAPPS "Enable test application" OFF)
option (TFLITE_ENABLE_GPU "" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_FIND_FRAMEWORK NEVER) # On MacOS, don't use framework for opencv
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/)

# iOS cannot use shared library since we would need to compile this with certificate and provisioning profile
# which is not possible in this context. Build as static library instead with all symbols visible (default behavior).
if (NOT APPLE)
  add_compile_options(
          -fPIC
          -ffunction-sections
          -fdata-sections
          -fvisibility=hidden
          $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>
          $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
          $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-float-conversion>
          $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>
          $<$<COMPILE_LANGUAGE:CXX>:-Wno-gnu-inline-cpp-without-extern>
  )
  add_link_options(
          -Wl,--gc-sections
          -Wl,--strip-all
  )
  set (${PROJECT_NAME}_LIBRARY_TYPE SHARED)
else()
  set (${PROJECT_NAME}_LIBRARY_TYPE STATIC)
endif()

# Dependencies
# ====================================================
include (configure-opencv)
add_subdirectory (3rdparty/cxlog)
add_subdirectory (3rdparty/opencv)
add_subdirectory (3rdparty/tensorflow/tensorflow/lite EXCLUDE_FROM_ALL)

# Library definition
# ====================================================
add_library (${PROJECT_NAME}
    ${${PROJECT_NAME}_LIBRARY_TYPE}
)
target_sources (${PROJECT_NAME}
        PRIVATE
          src/Image.cxx
          src/LocalInterpreter.cxx
          src/Processor.cxx
        PUBLIC FILE_SET HEADERS
        BASE_DIRS include/
        FILES
          include/tfcv/ml/localinterpreter.hpp
          include/tfcv/ml/processor.hpp
          include/tfcv/defs.hpp
          include/tfcv/image.hpp
          include/tfcv/utility.hpp

)
target_include_directories (${PROJECT_NAME}
        PUBLIC
          ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${OPENCV_INCLUDE_DIRS}
)
target_link_libraries (${PROJECT_NAME}
        PUBLIC
          cxlog
          opencv_core
          opencv_imgproc
          opencv_imgcodecs
          tensorflow-lite
          $<$<BOOL:${ANDROID}>:GLESv3>
          $<$<BOOL:${ANDROID}>:EGL>
          $<$<BOOL:${ANDROID}>:android>
)
install (TARGETS ${PROJECT_NAME}
        FILE_SET HEADERS
)

# Configure PODSPEC file
# ====================================================
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/ios.podspec
        ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.podspec
)
