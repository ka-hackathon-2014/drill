# setup cmake
cmake_minimum_required (VERSION 2.8)

# project metadata
project (drill)

# configure compiler
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall -Wextra -pedantic")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -mtune=native -march=native -Werror")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -g -ggdb")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-O1 -Wl,--hash-style=gnu -Wl,--sort-common -Wl,--demangle -Wl,--build-id")

# detect clang
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -ferror-limit=3 -fcolor-diagnostics -fdiagnostics-show-category=name -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-unused-parameter -Wno-exit-time-destructors -Wno-global-constructors -Wno-disabled-macro-expansion -Wno-weak-vtables")
endif ()

# set include paths
include_directories ("src")

# build libfluxcore
file (GLOB_RECURSE SourceFiles "src/*.cc")
add_executable (drill ${SourceFiles})
target_link_libraries (drill opencv_core opencv_highgui opencv_imgproc opencv_objdetect alure openal)

