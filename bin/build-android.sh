#!/bin/bash

platforms=('arm64-v8a' 'armeabi-v7a' 'x86' 'x86_64')

if [ -z "$ANDROID_HOME" ]; then
  echo "ANDROID_HOME is not set"
  exit 1
fi

set -e

# shellcheck disable=SC2145
echo "Building for Android...\nANDROID_HOME=$ANDROID_HOME\nplatforms=${platforms[@]}"

for arch in "${platforms[@]}"; do
  echo "Building for $arch..."

  cmake -DCMAKE_TOOLCHAIN_FILE="$ANDROID_HOME"/ndk/25.2.9519653/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI="$arch" \
        -DANDROID_PLATFORM=android-26 \
        -B"build.android.$arch" \
        -DCMAKE_INSTALL_PATH="lib/android/jniLibs/$arch" \
        -DBUILD_SHARED_LIBS=ON \
        $(test "$arch" = "armeabi-v7a" && echo "-DTFLITE_ENABLE_XNNPACK=OFF") \
        .

  cmake --build "build.android.$arch" --target tfcv --config Release -j 22

  mkdir -p "lib/android/jniLibs/$arch"
  cp "build.android.$arch/lib/libtfcv.so" "lib/android/jniLibs/$arch/libtfcv.so"
done