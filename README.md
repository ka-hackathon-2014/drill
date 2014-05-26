# drill

**Warning: This app may kick your ass!**

## Requirements
To run drill, you need at least the following environment:

* good Webcam (not to blurry, try to adjust the lighting)
* OpenCV 2
* OpenAL, Alure
* Linux, OSX (untested), Windows (untested)

You may want to use your package manager to get the required libs instead of downloading them manually.

## Build
In addition to the listed libraries, you need a modern C++ compiler which supports C++11, CMake and a build tool like Make or Ninja. You can simply build ninja by using:

    mkdir build
    cd build
    cmake -G Ninja ..
    ninja

To create release or debug builds, add `DCMAKE_BUILD_TYPE=Release` or `DCMAKE_BUILD_TYPE=Debug` to the CMake command line. You may also change the build tool to Make or Visual Studio. Please refer to the internet for more details :wink:

## Usage
To start drill, go to the top level source directory (not the build directory!) and start drill using:

    ./build/drill

## License

Copyright (C) 2014 the drill developers

Distributed under the MIT License.

