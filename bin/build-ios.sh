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
mkdir -p lib/ios/
libtool -static -o lib/ios/libtfcv-ios.a $(find build.ios.os/ -name '*.a')
libtool -static -o lib/ios/libtfcv-simulator.a $(find build.ios.simulator/ -name '*.a')
lipo -create lib/ios/libtfcv-ios.a lib/ios/libtfcv-simulator.a -output lib/ios/libtfcv.a