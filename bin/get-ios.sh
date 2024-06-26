#!/bin/sh

# Try to download artifact from GitHub
wget -O release.zip https://github.com/lmiklosko/tfcv/releases/download/v1.0.0/Multiplatform.zip

# If failed to do so, build from source
if [ $? -eq 0 ]; then
  unzip -o release.zip
  echo "iOS library downloaded successfully"
else
  echo "iOS library download failed, building from sources"
  bin/build-ios.sh
fi