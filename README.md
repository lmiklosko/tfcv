# TFCV (TensorFlow/OpenCV Prebuilt binaries)

#### This repository contains prebuilt binaries for TensorFlow and OpenCV alongside code to build them.

## You may ask; Why?
Over past few years, I have been working on various projects that require TensorFlow and OpenCV. I have been building 
these libraries from source for different platforms, and it is a time-consuming process. Whenever new version of these
are released, builds usually break, and it takes quite some time to fix them. Android builds are even more painful. 
CMake build system is famously broken in TensorFlow, and to this date has issues building GPU support, besides XNNPACK.

This repository aims to provide prebuilt binaries for these, so you can focus on your project rather than
taking your hair out trying to build these libraries.

## How to use?
Releases contain prebuilt binaries for different platforms. 

### Set up your project
#### Android
AAR files are provided for Android. You can include these in your Android project.
Optioanlly, you can also include the .SO files directly in your project by copying them under app/src/main/jniLibs and 
copy include files from include folder to your project's jni folder.

#### iOS
For iOS, static library is provided alongside headers. It exports all TensorFlow and OpenCV symbols (core, imgproc, 
and imgcodecs with PNG, JPEG support). Simply adding a podspec dependency should work. Optionally you can copy the library
and import it in your project.

## FAQ
### My codebase is using the interpreter API, but I would like to use custom interpreter code. How can I do that?
`ml::Processor::create()` is exported as a weak symbol in the library. You can override this symbol in your codebase
to supplement your custom interpreter code. This is particularly useful if you want to use Google Play Services ML Kit.


