import sys
sys.path.append(r"C:\Users\lauri\Desktop\ScreenLed\ScreenLed\pc")
from pc import analyseimg, shitpcanalyseimg
import cv2 as cv
import pickle
import numpy as np
import time


def preprocess_video(filepath):
    video = cv.VideoCapture(filepath)
    succ, image = video.read()
    analysis = {}
    frame = 0
    start_time = time.time()
    while succ:
        timestamp = video.get(cv.CAP_PROP_POS_MSEC)
        
        if (frame % 10) == 0:
            print("current analysing time:", int(timestamp), "/ frame:", frame)
        
        img = image[...,:3] #alphamap dump
        h_s = int(image.shape[0] / 3)
        h_e = int((image.shape[0] / 3) * 2)
        img = img[h_s:h_e]
        
        if timestamp != 0:
            #analysis[str(int(timestamp))] = analyseimg(image)
            analysis[str(int(timestamp))] = shitpcanalyseimg(image)

        frame += 1
        succ, image = video.read()

    #print(analysis)
    print("Done! took:", time.time()-start_time, "s")
    return analysis

if __name__ == "__main__":
    anal = preprocess_video(r"D:\Tiedostot\Live music\babymetal\SPREADSHEET_RIP\budokan10 9_10.mp4")
    pickle.dump(anal, open("testi.ppv", "wb"))