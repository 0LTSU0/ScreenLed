# ScreenLed

## Description
Simple program to control addressable rgb strip based off of what's visible on screen. Values in the code most likely require some tweaking before this works (amount of leds, screenshot area, ip address...). Also, in the code there are two versions of screenshotting function and the function that analyses the screenshots. The "shitpc"-variants should be used on lower end pc's to keep "framerate" high and delay low enough.

## Usage
1. Run rasb.py on raspberry pi with led-strip connected like this:
![Connections](https://tutorials-raspberrypi.de/wp-content/uploads/Raspberry-Pi-WS2812-Steckplatine-600x361.png)
2. Run pc.py on your computer 
