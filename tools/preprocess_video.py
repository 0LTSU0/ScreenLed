import sys
import cv2 as cv
import pickle
import numpy as np
import time
import pathlib

sys.path.append(str(pathlib.Path(__file__).parent.parent.absolute().joinpath("pc")))
from pc import analyseimg, shitpcanalyseimg


def preprocess_video(filepath):
    video = cv.VideoCapture(filepath)
    num_frames = video.get(cv.CAP_PROP_FRAME_COUNT)
    succ, image = video.read()
    analysis = {}
    frame = 0
    start_time = time.time()
    while succ:
        timestamp = video.get(cv.CAP_PROP_POS_MSEC)
        
        if (frame % 10) == 0:
            print("current analysing time:", int(timestamp), "| frame:", frame, "/", int(num_frames))
        img = cv.cvtColor(image, cv.COLOR_BGR2RGB)
        img = image[...,:3] #alphamap dump
        h_s = int(image.shape[0] / 3)
        h_e = int((image.shape[0] / 3) * 2)
        img = img[h_s:h_e] #Analyze only center 1/3 in case of black bars

        if timestamp != 0:
            #analysis[str(int(timestamp))] = analyseimg(image)
            analysis[str(int(timestamp))] = shitpcanalyseimg(image)

        frame += 1
        succ, image = video.read()

    print("Done! took:", (time.time()-start_time) / 60, "m")
    return analysis


if __name__ == "__main__":
    video_file = pathlib.Path(r"D:\Tiedostot\Live music\babymetal\BABYMETAL -「Elevator Girl (Japanese Ver.)」[Live Compilation] [字幕 _ SUBTITLED] [HQ].mp4")
    video_file = pathlib.Path(r"D:\Tiedostot\ledtestvid.mp4")
    output_file = video_file.parent.joinpath(video_file.stem + ".ppv")
    anal = preprocess_video(str(video_file.absolute()))
    pickle.dump(anal, open(output_file, "wb"))