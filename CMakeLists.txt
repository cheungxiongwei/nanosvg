cmake_minimum_required(VERSION 3.27.7)

project(
  NanoSVG
  VERSION 1.0.0
  DESCRIPTION "Embedded NanoSVG"
  HOMEPAGE_URL "https://github.com/cheungxiongwei/nanosvg"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# cmake-format: off
list(APPEND CMAKE_PREFIX_PATH "C:/Hub/Source/opencv/build/x64/vc16/lib")
list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)

find_package(OpenCV REQUIRED)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-int-conversion")

add_library(nanosvg src/nanosvg.c)
target_include_directories(nanosvg PUBLIC $<INSTALL_INTERFACE:include/nanosvg>)

add_library(nanosvgrast src/nanosvgrast.c)
target_link_libraries(nanosvgrast PUBLIC nanosvg)
target_include_directories(nanosvgrast PRIVATE src)
target_include_directories(nanosvgrast INTERFACE src)

add_executable(nanoembedded embedded/main.cpp
                            embedded/nanosparser.cpp
                            embedded/nanosparser.hpp
                            embedded/bezier.h
                            embedded/bezier.cpp
                            embedded/PerformanceTimer.hpp)

target_link_libraries(nanoembedded PRIVATE nanosvgrast)

# OpenCV
target_include_directories(nanoembedded PUBLIC ${OpenCV_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
target_link_libraries(nanoembedded PUBLIC ${OpenCV_LIBS})

# cmake-format: on
