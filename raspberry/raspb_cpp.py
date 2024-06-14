import socket
from sys import byteorder
from rpi_ws281x import *


LED_SEGMENTS = 20


class strip():
    def __init__(self, LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL, RGB, REVERSE):
        self.LED_COUNT = LED_COUNT
        self.LED_PIN = LED_PIN
        self.LED_FREQ_HZ = LED_FREQ_HZ
        self.LED_DMA = LED_DMA
        self.LED_BRIGHTNESS = LED_BRIGHTNESS
        self.LED_INVERT = LED_INVERT
        self.LED_CHANNEL = LED_CHANNEL
        self.RGB = RGB # TRUE == RGB, FALSE == BGR
        self.REVERSE = REVERSE
        
strip_dense = strip(287, 18, 800000, 10, False, 255, 0, True, True)
strip_old = strip(280, 13, 800000, 10, False, 255, 1, True, False)

stripcfgs = [strip_dense, strip_old]
strips = []

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
                        i = 0
                        for st in strips:
                            LED(ledifo, st, stripcfgs[i])
                            i += 1
            except Exception as e:
                print(e, ledifo)

    
def LED(data, strip, cfg):
    if cfg.REVERSE:
        j = len(data) - 3
    else:
        j = 0 #point in message
    leds_per_segment = int(cfg.LED_COUNT / LED_SEGMENTS)
    extra_leds = cfg.LED_COUNT % LED_SEGMENTS #TODO: use
    for k in range(LED_SEGMENTS):
        #if not cfg.REVERSE:        
        r, g, b = data[j], data[j+1], data[j+2]
        #else:
        #    r, g, b = data[j], data[j+1], data[j+2]
        add = 0
        #if extra_leds > 0: #put same color to one too many led until all extra leds are used
        #    add = 1
        #    extra_leds =- 1

        for i in range(leds_per_segment + add):
            if not cfg.RGB:
                strip.setPixelColor(i+k*leds_per_segment, Color(max(g-20, 0), max(b-20, 0), max(r-20, 0)))
            else:
                strip.setPixelColor(i+k*leds_per_segment, Color(max(r-20, 0), max(g-20, 0), max(b-20, 0)))

        #if k != 0:
        #if not cfg.RGB:
        #    strip.setPixelColor(k, Color(max(data[j+1]-20, 0), max(data[j+2]-20, 0), max(data[j]-20, 0)))
        #else:
        #    strip.setPixelColor(k, Color(max(data[j]-20, 0), max(data[j+1]-20, 0), max(data[j+2]-20, 0)))
        #for i in range(14): #1 when 20 leds!
        #    if not cfg.RGB:
        #        strip.setPixelColor(i+k*14+20, Color(max(data[j+1]-20, 0), max(data[j+2]-20, 0), max(data[j]-20, 0)))
        #    else:
        #        strip.setPixelColor(i+k*14+20, Color(max(data[j]-20, 0), max(data[j+1]-20, 0), max(data[j+2]-20, 0)))
        #    #h += 1
        if cfg.REVERSE:
            j -= 3
        else:
            j += 3
    strip.show()


for stc in stripcfgs:
    strips.append(Adafruit_NeoPixel(stc.LED_COUNT, stc.LED_PIN, stc.LED_FREQ_HZ, stc.LED_DMA, stc.LED_INVERT, stc.LED_BRIGHTNESS, stc.LED_CHANNEL))
for st in strips:
    st.begin()
func()
