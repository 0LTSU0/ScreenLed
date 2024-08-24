cmake_minimum_required(VERSION 3.14)

project(VideoCapture2LED LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

if (WIN32)
    set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/opencv_win/build/x64/vc16/lib")
else()
    set(CMAKE_C_COMPILER "/usr/bin/gcc")
    set(CMAKE_CXX_COMPILER "/usr/bin/g++")    

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WS2811 REQUIRED libws2811)
    include_directories(${WS2811_INCLUDE_DIRS})
    link_directories(${WS2811_LIBRARY_DIRS})
endif()

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/json)

add_executable(VideoCapture2LED
  main.cpp
  defs.h
  configurator.h
  configurator.cpp
)

message(STATUS "OpenCV Libraries: ${OpenCV_LIBS}")
target_link_libraries(VideoCapture2LED Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(VideoCapture2LED ${OpenCV_LIBS})
if(NOT WIN32)
    target_link_libraries(VideoCapture2LED ${WS2811_LIBRARIES})
endif()

include(GNUInstallDirs)
install(TARGETS VideoCapture2LED
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
