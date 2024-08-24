# The ultimate way of syncing screen to LEDs: Using capture card

This folder contains simle c++ program that can be used on Raspberry pi with a capture card. This is very much work in progress and currently assumes you have just one video device connected to raspi and there is no configurations for led strips.

## Building and running:
1. Install prerequisites:
    - Install opencv and qt libraries:
        1. sudo apt-get install libopencv-dev
        2. sudo apt-get install qt5-default
        3. sudo apt-get install qtdeclarative5-dev
    - Build and install rpi_ws281x:
        1. cd rpi_ws281x-master
        2. mkdir build
        3. cd build
        4. cmake -D BUILD_SHARED=ON -D BUILD_TEST=OFF ..
        5. cmake --build .
        6. sudo make install
2. Build application:
    1. mkdir build_raspi
    2. cmake ..
    3. make
3. Run (rpi_ws281x is supposed to be user space library but couldn't get it working without sudo)
    1. Connect capture card to rapsi with USB
    2. sudo ./VideoCapture2LED 