# drill


## Getting started

Required:

* OpenCV 2
* OpenAL, Alure
* C++ compiler (e.g. clang, modern C++11 environment)

Build drill:

    ninja

Or manually:

    clang++ -std=c++11 -O2 src/*.cc -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lalure -lopenal

Note: You may use pkg-config instead. You may use g++ (4.8+) instead.


## Usage

    ./drill

or specify OpenCV 2 classifier:

    ./drill classifier/haarcascade_frontalface_alt.xml


## Environment Variables

The following environment variables are recognized:

Environment variable   |  Type     |  Effect
---------------------- | --------- | -----------------
DRILL\_VERBOSE         | set/unset | Various debug messages
DRILL\_SHOWSELF        | set/unset | UI, for showing webcam and tracked feature
DRILL\_CAMTHRESHOLD    | double    | Filter movement jitter, based on percentage of face heigth
DRILL\_GUIFPS          | unsigned  | Frames per second, depends on DRILL\_SHOWSELF
DRILL\_CAMSLICELENGTH  | unsigned  | Sliding window timeslice in ms, for direction estimation

Note: set only if you know what you're doing!


## License

Copyright (C) 2014 the drill developers

Distributed under the MIT License.
