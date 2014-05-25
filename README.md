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


## License

Copyright (C) 2014 the drill developers

Distributed under the MIT License.
