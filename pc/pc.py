import time
import numpy as np
import cv2 as cv
import socket
from PIL import Image
import win32gui, win32ui, win32con
import json


HOST = '192.168.1.6'
#HOST = '127.0.0.1' #for local testing
PORT = 65432

framerate = 30
segs = 20
resx = 1920
resy = 1080

arrays = open("arrays.txt", "w")
#caputure and do processing
def mainpc(addr=None, port=None, epreview=None, slow_pc_mode=None):
    if addr and port and epreview != None and slow_pc_mode != None:
        global HOST, PORT
        HOST = addr
        PORT = port
    else:
        print("You're calling this function wrong?")
        return
    
    print("Trying to connect to {}:{}".format(addr, port))
    i = 0
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))
    while True:
        s_t = time.perf_counter() #start time of loop
        
        #take screenshot and covert to numpy array
        #image = pyautogui.screenshot()
        sss = time.perf_counter()
        
        if slow_pc_mode:
            image = shitpcss()
        else:
            image = ss()
        
        image = cv.cvtColor(image, cv.COLOR_BGR2RGB)
        sse = time.perf_counter()
        if epreview:
            cv.imshow('test', image)
            if cv.waitKey(1) == ord('q'):
                cv.destroyAllWindows()
                break

        ans = time.perf_counter()
        if slow_pc_mode:
            analysis = shitpcanalyseimg(image)
        else:
            analysis = analyseimg(image) #do analysis
        analysis.reverse()
        ane = time.perf_counter()

        # print((analysis[i]))
        sends = time.perf_counter()
        packet = bytes(0)
        for i in range(segs):
            x = int(analysis[i][0]).to_bytes(1, 'big')
            y = int(analysis[i][1]).to_bytes(1, 'big')
            z = int(analysis[i][2]).to_bytes(1, 'big')
            appendstr = x + y + z
            packet = packet + appendstr

        sock.sendall(packet)
        sende = time.perf_counter()

        e_t = time.perf_counter() #end time of loop
        
        #print('screenshot: ' + str(sse-sss))
        #print('processing: ' + str(ane - ans))
        #print('sending: ' + str(sende - sends))
        fpsstr = "fps: " + str(1 / (e_t-s_t))
        print("\r" + fpsstr, end="")

        #Only sleep if need be (not likely ever)
        if (e_t - s_t < 1/(framerate + 10)):
            #print("sleeping")
            time.sleep(float(1/(framerate + 10)) - (e_t- s_t))
        i += 1


def analyseimg(img):
    segwidth = int(resx/segs)
    img = cv.resize(img, (1280, 720), interpolation = cv.INTER_AREA)
    #cv.imshow('test', img)
    #if cv.waitKey(1) == ord('q'):
    #    cv.destroyAllWindows()
    splitted = np.hsplit(img, segs)
    result = []
    for i in range(segs):
        m = np.mean(splitted[i], axis=(0,1), dtype='int')
        result.append(m)
    return result

def shitpcanalyseimg(img):
    segwidth = int(resx/segs)
    img = cv.resize(img, (960, 360), interpolation = cv.INTER_AREA)
    splitted = np.hsplit(img, segs)
    result = []
    for i in range(segs):
        m = np.mean(splitted[i], axis=(0,1), dtype='int')
        result.append(m)
    return result


#Function to take screenshot using win32 api
def ss():
    w = resx
    h = resy
    hwnd = None #window to capture
    wDC = win32gui.GetWindowDC(hwnd)
    dcObj = win32ui.CreateDCFromHandle(wDC)
    cDC = dcObj.CreateCompatibleDC()
    dataBitMap = win32ui.CreateBitmap()
    dataBitMap.CreateCompatibleBitmap(dcObj, w, h)
    cDC.SelectObject(dataBitMap)
    cDC.BitBlt((0,0), (w,h), dcObj, (0,0), win32con.SRCCOPY)

    #dataBitMap.SaveBitmapFile(cDC, 'debug.bmp')
    signedIntsArray = dataBitMap.GetBitmapBits(True)
    img = np.fromstring(signedIntsArray, dtype='uint8')
    img.shape = (h, w, 4)
    img = img[...,:3] #dump alphamap from bitmap

    dcObj.DeleteDC()
    cDC.DeleteDC()
    win32gui.ReleaseDC(hwnd, wDC)
    win32gui.DeleteObject(dataBitMap.GetHandle())

    return img

#screenshot only 1/3 of screen to speed screenshotting process up
def shitpcss():
    w = resx
    h = int((resy/3))
    hwnd = None #window to capture
    wDC = win32gui.GetWindowDC(hwnd)
    dcObj = win32ui.CreateDCFromHandle(wDC)
    cDC = dcObj.CreateCompatibleDC()
    dataBitMap = win32ui.CreateBitmap()
    dataBitMap.CreateCompatibleBitmap(dcObj, w, h)
    cDC.SelectObject(dataBitMap)
    cDC.BitBlt((0,0), (w,h), dcObj, (0,h), win32con.SRCCOPY)

    #dataBitMap.SaveBitmapFile(cDC, 'debug.bmp')
    signedIntsArray = dataBitMap.GetBitmapBits(True)
    img = np.fromstring(signedIntsArray, dtype='uint8')
    img.shape = (h, w, 4)
    img = img[...,:3] #dump alphamap from bitmap

    dcObj.DeleteDC()
    cDC.DeleteDC()
    win32gui.ReleaseDC(hwnd, wDC)
    win32gui.DeleteObject(dataBitMap.GetHandle())

    return img

if __name__ == "__main__":
    print("Running single threaded")
    with open("conf.json", "r") as f:
        conf = json.load(f)
        addr = conf["address"]
        port = conf["port"]
        preview = conf["enable_preview"]
        slow_pc = conf["enable_shitpc"]
    
    mainpc(addr=addr, port=port, epreview=preview, slow_pc_mode=slow_pc)
