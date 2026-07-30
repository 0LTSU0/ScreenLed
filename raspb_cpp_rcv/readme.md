1. Install led library on raspi:
    - git clone https://github.com/jgarff/rpi_ws281x.git
    - cd rpi_ws281x
    - mkdir build
    - cd build
    - cmake -D BUILD_SHARED=ON -D BUILD_TEST=OFF ..
    - cmake --build .
    - sudo make install

2. Build and run
    - g++ led_rcv.cpp -o led_receiver -lws2811 -lpthread
    - sudo ./led_receiver