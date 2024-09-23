#!/bin/bash

set -e

TARGET_DIR="platform/android/tfcv/src/cxx/include"

cp -r 3rdparty/opencv/modules/core/include/* $TARGET_DIR/opencv/
cp -r 3rdparty/opencv/modules/imgproc/include/* $TARGET_DIR/opencv/
cp -r 3rdparty/opencv/modules/imgcodecs/include/* $TARGET_DIR/opencv/

cp -r 3rdparty/opencv/modules/core/include/* $TARGET_DIR/tfcv/
cp -r 3rdparty/opencv/modules/imgproc/include/* $TARGET_DIR/tfcv/
cp -r 3rdparty/opencv/modules/imgcodecs/include/* $TARGET_DIR/tfcv/
cp -r 3rdparty/cxlog/include/cxlog $TARGET_DIR/tfcv/.
cp -r include/tfcv $TARGET_DIR/tfcv/.

cd platform/android
./gradlew assembleRelease