Pod::Spec.new do |spec|
  spec.name         = "tfcv"
  spec.version      = "0.1.0"
  spec.summary      = "Pre-compiled tensorflow lite for computer vision"
  spec.description  = "Pre-compiled tensorflow lite for computer vision"
  spec.homepage     = "https://github.com/lmiklosko/tfcv.git"

  spec.static_framework = true
  spec.dependency 'TensorFlowLiteObjC/CoreML'

  spec.license      = "MIT"
  spec.authors      = { "Lukas Miklosko" => "l.miklosko@gmail.com" }

  spec.platform     = :ios, "13.0"
  spec.source       = { :git => "https://github.com/lmiklosko/tfcv.git" }

  spec.source_files         = "platform/ios/public/"
  spec.public_header_files  = "platform/ios/public/*.h"

  spec.library              = "c++"
  spec.vendored_libraries   = 'lib/ios/libtfcv.a'

  # ――― Project Settings ――――――――――――――――――――――――――――――――――――――――――――――――――――――――― #
  spec.xcconfig             = {
        "HEADER_SEARCH_PATHS" => "/#{spec.name}/**",
        'CLANG_CXX_LANGUAGE_STANDARD' => 'c++20',
        'CLANG_CXX_LIBRARY' => 'libc++'
  }
  spec.prepare_command  = "sh bin/build-ios.sh"
end
