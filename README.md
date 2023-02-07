# Status of last deployment
![Build libvideo](https://github.com/tomsksoft/libvideo/actions/workflows/pull_request.yml/badge.svg) <br>
![Release libvideo](https://github.com/tomsksoft/libvideo/actions/workflows/release.yml/badge.svg)

# libvideo
libvideo

## About
libvideo is crossplatform open-source library for encoding and decoding RGBA video using the H.264 standard.
The library supports scaling and cropping incoming and outgoing frames.

## License
```
libvideo
Copyright (C) 2022 Tomsksoft

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
```

This software uses code of [FFmpeg](http://ffmpeg.org) licensed under the [LGPLv2.1](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
and code of [libx264](https://www.videolan.org/developers/x264.html) licensed under the [GPLv2.0](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html). <br>
[FFmpeg source](https://github.com/FFmpeg/FFmpeg), [libx264 source](https://code.videolan.org/videolan/x264)

libvideo owners are not the owners of FFmpeg and libx264. You can go to the site with the owners using the links above.

libvideo licensed under the [GPLv2.0](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html) (more details in the LICENSE file)
and owned by the [TomskSoft](https://tomsksoft.com/) company.

## Platforms
- Windows 10
- Android API 21

## Requirements
- CMake 3.16.0
- Conan 1.56.0
- Visual Studio 2019 (Windows only)
- Linux (Cross build Linux to Android)

# Windows build

## Before build libvideo on Windows
On main project directory execute:<br>
debug version
```
py .ci/prebuild_thirdparty.py --build-debug
```
release version
```
py .ci/prebuild_thirdparty.py --build-release
```

## Build via Visual Studio
```
cmake -S . -B build_ -G "Visual Studio 16 2019"
MSBuild /fl /flp:Verbosity=diagnostic /t:Build /p:Configuration=Debug /nologo /m:4 libvideo.sln
```
Use **-pr=.ci/conan_profiles/conan_profile_release** if you want build release version

## Build via Conan
```
conan build . --build-folder build_
```

## Deployment Windows
```
conan create . libvideo/1.0.0@ccor/libvideo --build=missing
```

## Deployment Linux (Cross build Linux to Android)
```
conan create . libvideo/1.0.0@ccor/libvideo -tf None --build=missing
```

# Android build (cross-build from Linux)

## Before build libvideo on Linux
On main project directory execute:<br>
debug version
```
python3 .ci/prebuild_thirdparty.py --build-debug --android
```
release version
```
python3 .ci/prebuild_thirdparty.py --build-release --android
```

## Build via Android Studio
1) Open libvideo-android folder as Android Studio project
2) Use Android Studio to build project

## Build via Gradle
In AndroidSDK folder
```
./gradlew build
```

# Hardware acceleration

## Hardware encoding through AMD AMF (Version 1.4.26)

1) Go to [AMF SDK](https://github.com/GPUOpen-LibrariesAndSDKs/AMF) github and download project files

2) Create folder "ForAMF", inside this folder create folder "AMF"

3) Download [AMF SDK](https://github.com/GPUOpen-LibrariesAndSDKs/AMF)

4) Copy folders "components" and "core" from downloaded "amf/public/include" to "AMF" folder

    The path should be like this:
    ```
    C:/ForAMF/AMF/components
    C:/ForAMF/AMF/core
    ```
5) On main project directory execute:<br>
    debug version
    ```
    py .ci/prebuild_thirdparty.py --build-debug --enable-amf -cflags <path_to_AMF>
    ```
    release version
    ```
    py .ci/prebuild_thirdparty.py --build-release --enable-amf -cflags <path_to_AMF>
    ```

    For example:
    ```
    py .ci/prebuild_thirdparty.py --build-release --enable-amf -cflags C:/ForAMF/
    ```

## Formatting and code style
C++ code formatted by [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and using _clang-format. <br>
libvideo.clang-format based on LLVM format. <br>

For reformatting project files open main project folder and put command: <br>

Windows
```
clang-format.exe -i -style=file Include/*.h src/*.h src/*.cpp src/Filters/*.h src/Filters/*.cpp src/FormatConvertors/*.h src/FormatConvertors/*.cpp src/WindowsManager/*.h src/WindowsManager/*.cpp test_package/*.cpp test_package/*.h
```
Linux
```
clang-format -i -style=file Include/*.h src/*.h src/*.cpp src/Filters/*.h src/Filters/*.cpp src/FormatConvertors/*.h src/FormatConvertors/*.cpp src/WindowsManager/*.h src/WindowsManager/*.cpp test_package/*.cpp test_package/*.h
```

## Static code analyzing
C++ code analyzing by [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) and using .clang-tidy. <br>
For analyze code while build add `--enable-tidy` to prebuild thirdparty stage. <br>
For example: <br>
```
py .ci/prebuild_thirdparty.py --build-debug --enable-tidy
```
