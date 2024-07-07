Pod::Spec.new do |spec|
  spec.name         = "tfcv"
  spec.version      = "1.1.4"
  spec.summary      = "Pre-compiled tensorflow lite for computer vision"
  spec.description  = "Pre-compiled tensorflow lite for computer vision"
  spec.homepage     = "https://github.com/lmiklosko/tfcv"

  spec.static_framework = true
  spec.dependency 'TensorFlowLiteObjC/CoreML'

  spec.license      = "MIT"
  spec.authors      = { "Lukas Miklosko" => "l.miklosko@gmail.com" }

  spec.platform     = :ios, "13.0"
  spec.source       = { :git => "https://github.com/lmiklosko/tfcv" }

  spec.source_files         = "platform/ios/public/**/*.{h,m,mm}", "include/cxlog/**/*.hpp", "include/opencv2/**/*.{h,hpp}", "include/tfcv/**/*.hpp"
  spec.public_header_files  = "platform/ios/public/**/*.h", "include/tfcv/**/*.hpp", "include/cxlog/**/*.hpp", "include/opencv2/**/*.{hpp,h}"
  spec.header_mappings_dir  = "include"
  spec.preserve_paths       = "include"

  spec.library              = "c++"
  spec.vendored_libraries   = 'lib/libtfcv.a'

  spec.pod_target_xcconfig  = {
    'HEADER_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/include $(PODS_TARGET_SRCROOT)/platform/ios/public',
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++20',
    'CLANG_CXX_LIBRARY' => 'libc++'
  }
  spec.prepare_command  = "sh bin/get-ios.sh"

  # ――― Sub-specs ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――――― #
#   spec.subspec 'cv' do |cv|
#     cv.public_header_files  = "include/opencv4/**/*.h"
#     cv.vendored_libraries   = 'lib/ios/libopencv.a'
#     cv.xcconfig             = {
#         'HEADER_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/include/opencv4',
#         'CLANG_CXX_LANGUAGE_STANDARD' => 'c++20',
#         'CLANG_CXX_LIBRARY' => 'libc++'
#     }
#   end
#
#   spec.subspec 'cxl' do |cxl|
#     cxl.public_header_files  = "include/cxlog1/**/*.h"
#     cxl.vendored_libraries   = 'lib/ios/libcxlog.a'
#     cxl.xcconfig             = {
#         'HEADER_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/include/cxlog1',
#         'CLANG_CXX_LANGUAGE_STANDARD' => 'c++20',
#         'CLANG_CXX_LIBRARY' => 'libc++'
#     }
#   end
end
