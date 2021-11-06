import pyautogui
import time
import numpy as np
import cv2 as cv
import socket

HOST = '192.168.1.3'
#HOST = '127.0.0.1'
PORT = 65432

framerate = 30
segs = 20
resx = 1920
resy = 1080

arrays = open("arrays.txt", "w")
#caputure and do processing
def main():
    i = 0
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))
    while True:
        s_t = time.perf_counter() #start time of processing
        
        #take screenshot and covert to numpy array
        image = pyautogui.screenshot()
        image = np.array(image)

        analysis = analyseimg(image) #do analysis

        # print((analysis[i]))
        packet = bytes(0)
        for i in range(segs):
            x = int(analysis[i][0]).to_bytes(1, 'big')
            y = int(analysis[i][1]).to_bytes(1, 'big')
            z = int(analysis[i][2]).to_bytes(1, 'big')
            appendstr = x + y + z
            packet = packet + appendstr
        
        sock.sendall(packet)
        
        e_t = time.perf_counter() #end time of processing
        print("fps: " + str(1 / (e_t-s_t)))
        #Only sleep if need be (not likely ever)
        if (e_t - s_t < 1/(framerate + 10)):
            print("sleeping")
            time.sleep(float(1/(framerate + 10)) - (e_t- s_t))
        i += 1


def analyseimg(img):
    segwidth = int(resx/segs)
    splitted = np.hsplit(img, segs)
    result = []
    for i in range(segs):
        m = np.mean(splitted[i], axis=(0,1), dtype='int')
        result.append(m)
    return result

if __name__ == "__main__":
    main()