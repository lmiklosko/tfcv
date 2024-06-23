option (BUILD_SHARED_LIBS OFF)
option (BUILD_TESTS OFF)
option (ENABLE_PRECOMPILED_HEADERS OFF)
option (BUILD_PERF_TESTS OFF)
option (ENABLE_PIC FALSE)
option (CV_TRACE OFF)
option (WITH_1394 OFF)
option (WITH_ADE OFF)
option (WITH_VTK OFF)
option (WITH_EIGEN OFF)
option (WITH_FFMPEG OFF)
option (WITH_GSTREAMER OFF)
option (WITH_GTK OFF)
option (WITH_GTK_2_X OFF)
option (WITH_IPP OFF)
option (WITH_JASPER OFF)
option (WITH_JPEG ON)
option (WITH_WEBP OFF)
option (WITH_OPENEXR OFF)
option (WITH_OPENGL OFF)
option (WITH_OPENVX OFF)
option (WITH_OPENNI OFF)
option (WITH_OPENNI2 OFF)
option (WITH_PNG ON)
option (WITH_TBB OFF)
option (WITH_TIFF OFF)
option (WITH_V4L OFF)
option (WITH_OPENCL OFF)
option (WITH_OPENCL_SVM OFF)
option (WITH_OPENCLAMDFFT OFF)
option (WITH_OPENCLAMDBLAS OFF)
option (WITH_GPHOTO2 OFF)
option (WITH_LAPACK OFF)
option (WITH_ITT OFF)
option (WITH_QUIRC OFF)
option (WITH_MSMF OFF)
option (WITH_VFW OFF)
option (BUILD_ZLIB OFF)
option (BUILD_opencv_apps OFF)
option (BUILD_opencv_calib3d OFF)
option (BUILD_opencv_dnn OFF)
option (BUILD_opencv_features2d ON)
option (BUILD_opencv_flann OFF)  # No bindings provided. This module is used as a dependency for other modules.
option (BUILD_opencv_gapi OFF)
option (BUILD_opencv_ml OFF)
option (BUILD_opencv_photo ON)
option (BUILD_opencv_imgcodecs ON)
option (BUILD_opencv_shape OFF)
option (BUILD_opencv_videoio OFF)
option (BUILD_opencv_videostab OFF)
option (BUILD_opencv_highgui OFF)
option (BUILD_opencv_superres OFF)
option (BUILD_opencv_stitching OFF)
option (BUILD_opencv_java OFF)
option (BUILD_opencv_java_bindings_generator OFF)
option (BUILD_opencv_js OFF)
option (BUILD_opencv_python2 OFF)
option (BUILD_opencv_python3 OFF)
option (BUILD_opencv_python_bindings_generator OFF)
option (BUILD_EXAMPLES OFF)
option (BUILD_PACKAGE OFF)
option (BUILD_ANDROID_PROJECTS OFF)
option (BUILD_ANDROID_EXAMPLES OFF)
option (INSTALL_ANDROID_EXAMPLES OFF)


# Force no examples / opencv applications
set (BUILD_EXAMPLES OFF)
set (BUILD_opencv_apps OFF)
set (BUILD_PROTOBUF OFF)

SET (BUILD_opencv_imgcodecs ON)
SET (WITH_PNG ON)
SET (WITH_JPEG ON)


if (CMAKE_BUILD_TYPE STREQUAL Debug AND NOT CMAKE_SYSTEM_NAME STREQUAL "Android" AND NOT IOS)
  if (NOT APPLE)
    set (WITH_GTK ON)
    set (WITH_GTK_2_X ON)
  endif()
  set (BUILD_opencv_highgui ON)
  set (BUILD_opencv_videoio ON)
endif()


set (OPENCV_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/calib3d/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/core/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/dnn/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/features2d/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/flann/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/gapi/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/highgui/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/imgcodecs/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/imgproc/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/ml/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/objc/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/objdetect/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/photo/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/python/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/stitching/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/ts/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/video/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/videoio/include/
        ${PROJECT_SOURCE_DIR}/3rdparty/opencv/modules/world/include/
)