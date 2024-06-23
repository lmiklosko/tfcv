#!/bin/sh

version="13.0"
configuration="Release"

# build for mobile
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/modules/iOS.cmake -DIOS_PLATFORM=OS -DIOS_DEPLOYMENT_TARGET=${version} -H. -Bbuild.ios.os -GXcode
cmake --build build.ios.os/ --config ${configuration} -j10

# build for simulator
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/modules/iOS.cmake -DIOS_PLATFORM=SIMULATOR -DIOS_DEPLOYMENT_TARGET=${version} -H. -Bbuild.ios.simulator -GXcode
cmake --build build.ios.simulator/ --config ${configuration} -j10

# join libraries together
mkdir -p lib/ios/
lipo -create build.ios.os/lib/libtfcv.a build.ios.simulator/lib/libtfcv.a -output lib/ios/libtfcv.a