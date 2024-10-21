#!/bin/sh

# Try to download artifact from GitHub
wget -O release.zip https://syngenta-of-pest-detection.s3.eu-central-1.amazonaws.com/releases/ios/1.3.2.zip

# If failed to do so, build from source
if [ $? -eq 0 ]; then
  unzip -o release.zip
  echo "iOS library downloaded successfully"
else
  echo "iOS library download failed, building from sources"
  bin/build-ios.sh
fi
