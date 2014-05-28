# drill

**Warning: This app may kick your ass!**

**Note: Currently we only support German sounds.**

This implements a drill sergeant which watches you doing squats and shouts at you if you're weak.

## Requirements
To run drill, you need at least the following environment:

* Good webcam (not too blurry, try to adjust the lighting)
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

To create release or debug builds, add `-DCMAKE_BUILD_TYPE=Release` or `-DCMAKE_BUILD_TYPE=Debug` to the CMake command line. You may also change the build tool to Make or Visual Studio. Please refer to the internet for more details :wink:

## Usage
To start drill, go to the top level source directory (not the build directory!) and start drill using:

    ./build/drill

## Licenses

Copyright (C) 2014 the drill developers

The `classifier` directory which contains copies of the original OpenCV project. License and code can be found in the [OpenCV GitHub repo](https://github.com/Itseez/opencv).

The sounds in the `sound` directory and sub-directories are distributed under [CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/).

The source code and the remaining parts are distributed under the MIT License.

