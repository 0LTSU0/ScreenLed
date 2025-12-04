import network
import socket
import neopixel
import machine
import time
import sys
import esp32

# ---------------------------
# --- WIFI CONFIGURATION ---
# ---------------------------
WIFI_SSID = "ASUS"
WIFI_PASS = "001EAB040046"
STATIC_IP = "192.168.1.123"   # desired static IP
GATEWAY = "192.168.1.1"
SUBNET = "255.255.255.0"
DNS = "8.8.8.8"

LED_SEGMENTS = 20
HOST = "0.0.0.0"
PORT = 55555

# -----------------------
# Built-in status LED config
# -----------------------
STATUS_LED_PIN = 2         # commonly the built-in LED (blue) on many ESP32 dev boards
STATUS_LED_ACTIVE_HIGH = True  # set False if LED is active-low on your board

def led_on(pin):
    pin.value(1 if STATUS_LED_ACTIVE_HIGH else 0)

def led_off(pin):
    pin.value(0 if STATUS_LED_ACTIVE_HIGH else 1)

def led_toggle(pin):
    pin.value(0 if pin.value() else 1)

# ========================
# --- STRIP CONFIGS ---
# ========================
class Strip:
    def __init__(self, count, pin, rgb=True, reverse=False, brightness=1.0):
        self.LED_COUNT = count
        self.LED_PIN = pin
        self.RGB = rgb
        self.REVERSE = reverse
        self.BRIGHTNESS = brightness

strips = [Strip(100, 12, rgb=True, reverse=False),
          Strip(100, 12, rgb=True, reverse=False),
          Strip(100, 12, rgb=True, reverse=False)]


# -----------------------
# Wi-Fi connection (with status LED)
# -----------------------
status_led = machine.Pin(STATUS_LED_PIN, machine.Pin.OUT)
led_off(status_led)

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)

    if STATIC_IP:
        wlan.ifconfig((STATIC_IP, SUBNET, GATEWAY, DNS))

    print("Connecting to Wi-Fi '{}' ...".format(WIFI_SSID))
    wlan.connect(WIFI_SSID, WIFI_PASS)

    # Blink LED while connecting
    blink = True
    try:
        while not wlan.isconnected():
            # Toggle LED to indicate connection attempt
            led_toggle(status_led)
            time.sleep(0.4)
        # Connected: set LED solid on
        led_on(status_led)
        ip = wlan.ifconfig()[0]
        print("Connected. IP:", ip)
        return wlan
    except Exception as e:
        # if something goes wrong, ensure LED off
        led_off(status_led)
        raise

# ========================
# --- SETUP FULL STRIP ---
# ========================
total_leds = sum([st.LED_COUNT for st in strips])
pin = machine.Pin(strips[0].LED_PIN, machine.Pin.OUT)
full_strip = neopixel.NeoPixel(pin, total_leds)
led_buffer = bytearray(total_leds * 3)

# ========================
# --- LED HANDLER ---
# ========================
def clamp(v, lo=0, hi=255):
    return max(lo, min(hi, v))

def LEDS(data, st, index):
    global led_buffer
    leds_per_segment = st.LED_COUNT // LED_SEGMENTS
    skip_leds = sum(s.LED_COUNT for s in strips[:index])
    j = len(data)-3 if st.REVERSE else 0

    for k in range(LED_SEGMENTS):
        r, g, b = data[j], data[j+1], data[j+2]

        # Dark cutoff & boost
        r = clamp(r+20 if r>160 else r-20)
        g = clamp(g+20 if g>160 else g-20)
        b = clamp(b+20 if b>160 else b-20)

        for l in range(leds_per_segment):
            pos = skip_leds + k*leds_per_segment + l
            if pos >= total_leds: continue
            idx = pos*3
            if st.RGB:
                led_buffer[idx]   = r
                led_buffer[idx+1] = g
                led_buffer[idx+2] = b
            else:
                led_buffer[idx]   = g
                led_buffer[idx+1] = r
                led_buffer[idx+2] = b

        j -= 3 if st.REVERSE else -3

# ========================
# --- MAIN UDP SERVER ---
# ========================
def main():
    global led_buffer
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((HOST, PORT))
    sock.setblocking(False)
    print("Listening on port", PORT)

    latest_packet = b""
    while True:
        # Drain socket buffer, keep only newest packet
        packets_read = 0
        while True:
            try:
                data, addr = sock.recvfrom(2048)
                latest_packet = data
                packets_read += 1
            except OSError:
                break

        if not latest_packet:
            time.sleep(0.01)
            continue
        elif packets_read > 1:
            print("Skipped data packets:", packets_read - 1)

        try:
            d = latest_packet.decode()
        except Exception as e:
            print("Decode error:", e)
            continue

        parts = [p for p in d.split(";") if p]
        parts.reverse()
        ledifo = []
        for p in parts:
            for val in p.split(","):
                val = val.strip()
                if val: ledifo.append(int(val))

        if len(ledifo) != LED_SEGMENTS*3:
            continue

        for i, st in enumerate(strips):
            LEDS(ledifo, st, i)
        
        for i in range(total_leds):
            idx = i*3
            full_strip[i] = (led_buffer[idx], led_buffer[idx+1], led_buffer[idx+2])

        full_strip.write()

# ========================
# --- EXECUTION ---
# ========================
try:
    connect_wifi()
    main()
except KeyboardInterrupt:
    print("Program stopped.")