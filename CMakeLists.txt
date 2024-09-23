cmake_minimum_required(VERSION 3.23)
cmake_policy(SET CMP0065 NEW)

# Project settings
# ====================================================
project (tfcv
        VERSION 1.1.3
        DESCRIPTION "Pre-compiled tensorflow lite for computer vision"
        HOMEPAGE_URL "https://github.com/lmiklosko/tfcv"
)

option (BUILD_TESTAPPS "Enable test application" OFF)
option (TFLITE_ENABLE_GPU "" ON)
option (tfcv_LIBRARY_TYPE "Build library as shared or static" SHARED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_FIND_FRAMEWORK NEVER) # On MacOS, don't use framework for opencv
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

# XNNPACK fails to compile on armeabi-v7a due to some assembly code compilation errors
if (ANDROID_ABI STREQUAL "armeabi-v7a")
  set (TFLITE_ENABLE_XNNPACK OFF)
endif ()

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/)

# Get rid of annoying OpenCV warnings
add_compile_options (
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-float-conversion>
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-gnu-inline-cpp-without-extern>
)

# For shared libraries, we need to hide symbols by default
if (BUILD_SHARED_LIBS OR ${PROJECT_NAME}_LIBRARY_TYPE STREQUAL "SHARED")
  # iOS cannot use shared library since we would need to compile this with certificate and provisioning profile
  # which is not possible in this context. Build as static library instead with all symbols visible (default behavior).
  if (IOS)
    message(FATAL_ERROR "Cannot build shared library on iOS")
  endif ()

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
  endif ()
  set (${PROJECT_NAME}_LIBRARY_TYPE SHARED)
else()
  set (${PROJECT_NAME}_LIBRARY_TYPE STATIC)
endif()

# Dependencies
# ====================================================

# CXLog
add_subdirectory (3rdparty/cxlog)

# OpenCV
include (configure-opencv)
add_subdirectory (3rdparty/opencv)

include (strip)
target_strip_symbols (opencv_core)
target_strip_symbols (opencv_imgproc)
target_strip_symbols (opencv_imgcodecs)

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
target_strip_symbols (${PROJECT_NAME})
set_target_properties (${PROJECT_NAME}
      PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
install (TARGETS ${PROJECT_NAME}
        FILE_SET HEADERS
        DESTINATION include/${PROJECT_NAME}${PROJECT_VERSION_MAJOR}
        INCLUDES DESTINATION include/${PROJECT_NAME}${PROJECT_VERSION_MAJOR}
)

# Library definition - Pure OpenCV
# ====================================================
include(target_dependencies)

# What we do here is to collect all object files from the dependencies and link them together
# into a single library. This is necessary on Android, where we need to encapsulate all dependencies
# into a single library. On other platforms, we can link against the individual libraries.
# NOTE: This is a workaround as gradle does not pick outputs from custom commands / custom targets
# which would be the proper way to handle this.
collect_dependencies(opencv_core)
list(APPEND opencv_DEPENDENCIES ${opencv_core_DEPENDENCIES})

collect_dependencies(opencv_imgproc)
list(APPEND opencv_DEPENDENCIES ${opencv_imgproc_DEPENDENCIES})

collect_dependencies(opencv_imgcodecs)
list(APPEND opencv_DEPENDENCIES ${opencv_imgcodecs_DEPENDENCIES})

foreach(dependency IN LISTS opencv_DEPENDENCIES)
  list(APPEND opencv-object-files $<TARGET_OBJECTS:${dependency}>)
endforeach()

if (TARGET carotene_objs)
  message(WARNING "Including carotene_objs")
  list(APPEND opencv-object-files $<TARGET_OBJECTS:carotene_objs>)
endif ()

add_library (opencv STATIC
        $<TARGET_OBJECTS:opencv_core>
        $<TARGET_OBJECTS:opencv_imgproc>
        $<TARGET_OBJECTS:opencv_imgcodecs>
        ${opencv-object-files}
)



# Build test applications and helper tools if requested
# ====================================================
if (BUILD_TESTAPPS)
  add_executable(testapp src/testapp.cxx)
  target_link_libraries(testapp opencv_core opencv_imgproc opencv_imgcodecs)
endif (BUILD_TESTAPPS)