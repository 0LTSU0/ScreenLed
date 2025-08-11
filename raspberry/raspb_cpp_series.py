import socket
from sys import byteorder
from rpi_ws281x import *

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

strip_dense = strip(287, 12, 800000, 10, False, 255, 0, True, True)
strip_old = strip(185, 12, 800000, 10, False, 255, 0, True, False)
strips = [strip_dense, strip_old]

HOST = '0.0.0.0'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)
LED_SEGMENTS = 20

def main():
    global full_strip
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((HOST, PORT))
    while True:
        data, addr = sock.recvfrom(1024)
        print(addr, "Received data:", data)
        try:
            if data:
                d = data.decode()
                ledifo = []
                d = d.split(";")
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
                        LEDS(ledifo, st, i)
                        i += 1
                    full_strip.show()
        except Exception as e:
            print(e, ledifo)

def LEDS(data, st, i):
    #print("LEDS", data, st, i)
    global full_strip
    if st.REVERSE:
        j = len(data) - 3
    else:
        j = 0
    leds_per_segment = int(st.LED_COUNT / LED_SEGMENTS)
    skip_leds = 0
    for ctr in range(i):
        skip_leds += strips[ctr].LED_COUNT

    for k in range(LED_SEGMENTS):
        r, g, b = data[j], data[j+1], data[j+2]
        # below we subtract 20 from the values to make the strip turn off when content is close to dark. However,
        # if content is bright we don't want to do that so if val > 160, add 20
        if r > 160:
            r += 20
        if g > 160:
            g += 20
        if b < 160:
            b += 20
        for l in range(leds_per_segment):
            if not st.RGB:
                full_strip.setPixelColor(skip_leds+k*leds_per_segment+l, Color(max(g-20, 0), max(b-20, 0), max(r-20, 0)))
            else:
                full_strip.setPixelColor(skip_leds+k*leds_per_segment+l, Color(max(r-20, 0), max(g-20, 0), max(b-20, 0)))

        if st.REVERSE:
            j -= 3
        else:
            j += 3
        

total_leds = 0
for st in strips:
    total_leds += st.LED_COUNT
full_strip = Adafruit_NeoPixel(total_leds, strips[0].LED_PIN, strips[0].LED_FREQ_HZ, strips[0].LED_DMA, False, 255, 0)
full_strip.begin()
main()