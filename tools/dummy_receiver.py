
import socket
import threading
import sys
import time
import pygame
import random

HOST = '127.0.0.1'
PORT = 65432
NUM_SEGS = 0
CURRENT_COLORS = []
LOCK = threading.Lock()

class recvThread(threading.Thread):
    def __init__(self):
        super(recvThread,self).__init__()
        self.connected = False

    def mainrcv(self):
        print("Starting UDP listening")
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((HOST, PORT))
        global CURRENT_COLORS
        while True:
            msg, addr = sock.recvfrom(1024)
            print(f"Received from {addr}: {msg}")
            with LOCK:
                CURRENT_COLORS = self.decode(msg)
                
    def decode(self, data):
        res = []
        data = data.decode().split(";")
        for i in range(NUM_SEGS):
            d = data[i].split(",")
            c = 0
            for item in d:
                if not item:
                    d[c] = "0"
                c += 1
            res.append((d[0], d[1], d[2]))
        return res
        

    def run(self):
        self.mainrcv()

    def stop(self):
        sys.exit()

class visTrehad(threading.Thread):
    def __init__(self):
        super(visTrehad,self).__init__()
        self.running = False
        self.screen = None
        self.clock = None
        self.sizescale = 80
        
    def setup(self):
        pygame.init()
        self.screen = pygame.display.set_mode((self.sizescale * NUM_SEGS, self.sizescale))
        self.clock = pygame.time.Clock()
        self.running = True

    def visualize(self):
        while self.running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.running = False

            for i in range(NUM_SEGS):
                #color = "#"+''.join([random.choice('0123456789ABCDEF') for j in range(6)])
                try:
                    with LOCK:
                        color = pygame.Color(int(CURRENT_COLORS[i][0]), int(CURRENT_COLORS[i][1]), int(CURRENT_COLORS[i][2]))
                    pygame.draw.rect(self.screen, color, pygame.Rect((i*self.sizescale), 0, self.sizescale, self.sizescale))
                except IndexError:
                    pass
            pygame.display.flip()

            self.clock.tick(120)  # limits FPS to 120
        
        sys.exit()

    def run(self):
        self.setup()
        self.visualize()

if __name__ == "__main__":
    #if len(sys.argv) == 1:
    #    print("Usage: python dummy_receiver.py [n led segments]")
    #else:
    recv_thread = recvThread()
    vis_thread = visTrehad()
    #NUM_SEGS = int(sys.argv[1])
    NUM_SEGS = 20
    CURRENT_COLORS = [["0", "0", "0"]] * NUM_SEGS
    print("init colors:", CURRENT_COLORS)
    recv_thread.start()
    vis_thread.start()
