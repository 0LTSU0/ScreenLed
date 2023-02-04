# ScreenLed

## Live Mode

### Description
Simple program to control addressable rgb strip based off of what's visible on screen. Values in the raspberry pi code (raspb.py) most likely require some tweaking before this works (amount of leds...). Relevant settings for pc side should be configurable through the gui.

### Demo
Demo video of this running with some music videos here: https://youtu.be/UZCBObS5qF0

### Usage
1. Disable audio on your raspberry pi:

![Disable audio](enableraspberryaudio.JPG)

2. Run rasb.py on raspberry pi with led-strip connected like this:

![Connections](https://tutorials-raspberrypi.de/wp-content/uploads/Raspberry-Pi-WS2812-Steckplatine-600x361.png)

3. Run pc/gui.py on your computer
    - single threaded: "python gui.py"
    - multi threaded: "python gui.py multith"
    
    
## Preprocess Mode

### Description
Taking screenshots for analysis is rather slow on python so there is also another option to preprosess a video and play it through the script. This has greatly better performance on slow computers!

### Usage
1. Process a video file with pc/preprocess_video.py (this will create a python pickle dump of the led information to a file in the same folder as source with the same filename)
2. Run raspb.py
3. Play processed video with pc/preprocess_videoplayer.py (the pickle dump is attempted to be found in the same folder as video file)
