import time
import numpy as np
import cv2 as cv
import socket
from PIL import Image
import win32gui, win32ui, win32con
import json
import threading
import queue
from pc import *
import sys

HOST = '192.168.1.6'
#HOST = '127.0.0.1' #for local testing
PORT = 65432
framerate = 60
segs = 20
resx = 1920
resy = 1080

ss_queue = queue.Queue(maxsize=5)

class screenshotThread(threading.Thread):
    def __init__(self, lowend):
        super(screenshotThread,self).__init__()
        self.lowendmode = lowend

    def run(self):
        while True:
            if not ss_queue.full():
                if self.lowendmode:
                    ss_queue.put(shitpcss())
                else:
                    ss_queue.put(ss())
                #print("added screenshot")
            else:
                time.sleep(1/framerate)
            #time.sleep(1 / target_fps)


class processThread(threading.Thread):
    def __init__(self, lowend, socket, epreview):
        super(processThread,self).__init__()
        self.lowendmode = lowend
        self.socket = socket
        self.epreview = epreview

    def run(self):
        prev_sent = 0
        while True:
            #print("length of ss_queue", ss_queue.qsize())
            if not ss_queue.empty():
                image = cv.cvtColor(ss_queue.get(), cv.COLOR_BGR2RGB)
                if self.epreview:
                    cv.imshow('img under analysis', image)
                    if cv.waitKey(1) == ord('q'):
                        cv.destroyAllWindows()
                        break
                if self.lowendmode:
                    analysis = shitpcanalyseimg(image)
                else:
                    analysis = analyseimg(image)
                analysis.reverse()

                packet = bytes(0)
                for i in range(segs):
                    x = int(analysis[i][0]).to_bytes(1, 'big')
                    y = int(analysis[i][1]).to_bytes(1, 'big')
                    z = int(analysis[i][2]).to_bytes(1, 'big')
                    appendstr = x + y + z
                    packet = packet + appendstr

                self.socket.sendall(packet)

                sent = time.perf_counter()
                fpsstr = "fps: " + str(1 / (sent-prev_sent))
                print("\r" + fpsstr, end="")
                prev_sent = sent
                #print("sent")
            else:
                time.sleep(1 / target_fps)
            #time.sleep(1 / target_fps)


def main(addr=None, port=None, epreview=None, slow_pc_mode=None, t_fps=0):
    if addr and port and epreview != None and slow_pc_mode != None and t_fps >= 0:
        global HOST, PORT, target_fps
        HOST = addr
        PORT = port
        target_fps = t_fps
    else:
        print("You're calling this function wrong?")
        return

    print("Trying to connect to {}:{}".format(addr, port))
    i = 0
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    ss_thread = screenshotThread(lowend=slow_pc_mode)
    analyse_thread = processThread(lowend=slow_pc_mode, socket=sock, epreview=epreview)
    ss_thread.start()
    analyse_thread.start()


if __name__ == "__main__":
    print("running multithreaded")
    with open("conf.json", "r") as f:
        conf = json.load(f)
        addr = conf["address"]
        port = conf["port"]
        preview = conf["enable_preview"]
        slow_pc = conf["enable_shitpc"]
        target_fps = conf["target_fps"]
    
    main(addr=addr, port=port, epreview=preview, slow_pc_mode=slow_pc, t_fps=int(target_fps) + 10)