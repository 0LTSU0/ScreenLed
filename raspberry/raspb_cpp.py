import socket
from sys import byteorder
from rpi_ws281x import *


LED_COUNT = 300
LED_PIN = 18
LED_FREQ_HZ = 800000
LED_DMA = 10
LED_BRIGHTNESS = 255
LED_INVERT = False
LED_CHANNEL = 0
LED_SEGMENTS = 20

HOST = '192.168.1.69'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

def func():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind((HOST, PORT))
    sock.listen()
    conn, addr = sock.accept()
    with conn:
        print('Connected by', addr)
        while True:
            sock.listen()
            data = conn.recv(1024)
            try:
                if data:
                    d = data.decode()
                    ledifo = []
                    d = d.split(",;")
                    d.reverse()
                    for item in d:
                        if not item:
                            continue
                        sp = item.split(",")
                        for it in sp:
                            ledifo.append(int(it))
                            #ledifo.append((int(sp[0]),int(sp[1]),int(sp[2])))
                        #print("using ledinfo", ledifo)
                    if len(ledifo) == LED_SEGMENTS * 3:
                        LED(ledifo)
            except Exception as e:
                print(e, ledifo)

    
def LED(data):
    segs = len(data)
    j = 0
    h=0
    for k in range(20):
        #if k != 0:
        strip.setPixelColor(k, Color(max(data[j]-20, 0), max(data[j+1]-20, 0), max(data[j+2]-20, 0)))
        for i in range(14): #1 when 20 leds!
            strip.setPixelColor(i+k*14+20, Color(max(data[j]-20, 0), max(data[j+1]-20, 0), max(data[j+2]-20, 0)))
            #h += 1
        j += 3
    strip.show()

strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
strip.begin()
func()
