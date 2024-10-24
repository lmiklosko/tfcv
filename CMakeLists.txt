cmake_minimum_required(VERSION 3.23)
cmake_policy(SET CMP0065 NEW)

# Project settings
# ====================================================
project (tfcv
        VERSION 1.3.4
        DESCRIPTION "Pre-compiled tensorflow lite for computer vision"
        HOMEPAGE_URL "https://github.com/lmiklosko/tfcv"
)

option (BUILD_TESTAPPS "Enable test application" OFF)
option (TFLITE_ENABLE_GPU "" ON)
option (${PROJECT_NAME}_LIBRARY_TYPE "Build library as shared or static" SHARED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_FIND_FRAMEWORK NEVER) # On MacOS, don't use framework for opencv
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/)

# Get rid of annoying OpenCV warnings
add_compile_options (
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-float-conversion>
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-gnu-inline-cpp-without-extern>
)

# XNNPACK fails to compile on armeabi-v7a due to some assembly code compilation errors
if (ANDROID_ABI STREQUAL "armeabi-v7a")
  set (TFLITE_ENABLE_XNNPACK OFF)
endif ()

# For shared libraries, we need to hide symbols by default
if (BUILD_SHARED_LIBS OR ${PROJECT_NAME}_LIBRARY_TYPE STREQUAL "SHARED")
  # iOS cannot use shared library since we would need to compile this with certificate and provisioning profile
  # which is not possible in this context. Build as static library instead with all symbols visible (default behavior).
  if (IOS)
    message(FATAL_ERROR "Cannot build shared library on iOS")
  endif ()

  # Shared libraries require Position Independent Code. Also strip objects and enable dead code elimination
  add_compile_options (
          -fPIC
          -fvisibility=hidden
          $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>
          $<$<NOT:$<PLATFORM_ID:Darwin>>:-ffunction-sections>
          $<$<NOT:$<PLATFORM_ID:Darwin>>:-fdata-sections>
          $<$<CONFIG:Release>:-O2>
  )

  if (APPLE)
    add_link_options (
            -Wl,-dead_strip
    )
  else ()
    add_link_options (
            -Wl,--gc-sections
            -Wl,--strip-all
    )
  endif (APPLE)
  set (${PROJECT_NAME}_LIBRARY_TYPE SHARED)
else()
  if (NOT IOS)
    add_compile_options(
            -fPIC
    )
  endif ()
  set (${PROJECT_NAME}_LIBRARY_TYPE STATIC)
endif()

include(target_dependencies)
include(strip)

# Dependencies
# ====================================================

# CXLog
add_subdirectory (3rdparty/cxlog)

# OpenCV
include (configure-opencv)
add_subdirectory (3rdparty/opencv)

# TensorFlow needs to be built statically
unset(BUILD_SHARED_LIBS CACHE)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory (3rdparty/tensorflow/tensorflow/lite EXCLUDE_FROM_ALL)

# Library definition - TensorFlow Computer Vision
# ====================================================
add_library (${PROJECT_NAME} ${${PROJECT_NAME}_LIBRARY_TYPE})
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
          include/tfcv/ml/classifier.hpp
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
set_target_properties (${PROJECT_NAME}
      PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_strip_symbols (${PROJECT_NAME})
install (TARGETS ${PROJECT_NAME}
        FILE_SET HEADERS
        DESTINATION include/${PROJECT_NAME}${PROJECT_VERSION_MAJOR}
        INCLUDES DESTINATION include/${PROJECT_NAME}${PROJECT_VERSION_MAJOR}
)

# OpenCV static library
# ====================================================
add_library(opencv_c INTERFACE)
target_link_libraries(opencv_c INTERFACE opencv_core opencv_imgproc opencv_imgcodecs)
collect_dependencies(opencv_c)

foreach(dependency IN LISTS opencv_c_DEPENDENCIES)
  list(APPEND opencv_OBJECT_FILES $<TARGET_OBJECTS:${dependency}>)
endforeach()

if (TARGET carotene_objs)
  list(APPEND opencv_OBJECT_FILES $<TARGET_OBJECTS:carotene_objs>)
endif ()

add_library (opencv STATIC ${opencv_OBJECT_FILES})

# Build test applications and helper tools if requested
# ====================================================
if (BUILD_TESTAPPS)
  add_executable(testapp
          src/testapp.cxx
  )
  target_link_libraries(testapp tfcv opencv_highgui)
endif (BUILD_TESTAPPS)