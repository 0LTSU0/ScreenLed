cmake_minimum_required(VERSION 3.5)
project(ScreenLedCLI VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(CMAKE_CXX_STANDARD 20)

add_executable(ScreenLedCLI main.cpp)

target_link_libraries(ScreenLedCLI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Add ScreenLed library and json header
add_subdirectory(../ScreenLedLib_2 ${CMAKE_CURRENT_BINARY_DIR}/lib)
target_link_libraries(ScreenLedCLI PRIVATE ScreenLedLib)
