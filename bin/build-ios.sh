#!/bin/sh

version="13.0"
configuration="Release"

set -e

# build for mobile
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/modules/iOS.cmake -DIOS_PLATFORM=OS -DIOS_DEPLOYMENT_TARGET=${version} -H. -Bbuild.ios.os -GXcode
cmake --build build.ios.os/ --config ${configuration} -j10

# build for simulator
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/modules/iOS.cmake -DIOS_PLATFORM=SIMULATOR -DIOS_DEPLOYMENT_TARGET=${version} -H. -Bbuild.ios.simulator -GXcode
cmake --build build.ios.simulator/ --config ${configuration} -j10

# join libraries together
mkdir -p lib
libtool -static -o lib/libtfcv-ios.a $(find build.ios.os/ -name '*.a')
libtool -static -o lib/libtfcv-simulator.a $(find build.ios.simulator/ -name '*.a')
lipo -create lib/libtfcv-ios.a lib/libtfcv-simulator.a -output lib/libtfcv.a

# generate package
mkdir -p build.ios/{lib,include}
cp lib/libtfcv-ios.a build.ios/lib/libtfcv.a

TARGET_DIR=build.ios/include
cp -r 3rdparty/opencv/modules/core/include/* $TARGET_DIR/
cp -r 3rdparty/opencv/modules/imgproc/include/* $TARGET_DIR/
cp -r 3rdparty/opencv/modules/imgcodecs/include/* $TARGET_DIR/
cp -r 3rdparty/cxlog/include/cxlog $TARGET_DIR/.
cp -r include/* $TARGET_DIR/.

cd build.ios
zip -r release.zip .
aws s3 cp release.zip s3://syngenta-of-pest-detection/releases/ios/1.4.0.zip
