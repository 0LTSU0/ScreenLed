import socket
import tkinter as tk
import sys
import random
import time
import threading


HOST = '127.0.0.1'
PORT = 65432
NUM_SEGS = 0
CURRENT_COLORS = []


def update_window():
    #print("update_window()")
    if len(CURRENT_COLORS) != NUM_SEGS:
        print("Length of incoming data does not match to amount of segments!")
        return
    
    for i in range(NUM_SEGS)[::-1]:
        color = "#" + hex(int(CURRENT_COLORS[i][0]))[2:].zfill(2) + hex(int(CURRENT_COLORS[i][1]))[2:].zfill(2) + hex(int(CURRENT_COLORS[i][2]))[2:].zfill(2)
        #color = "#" + "%06x" % random.randint(0, 0xFFFFFF)
        label = tk.Label(text="", fg="white", bg=color, width=12, height=5)
        label.grid(row = 0, column = i)
    window.after(100, update_window) # Limit fps very slow or windows will throw a tantrum


def main(led_segs):
    window.title("DummyReceiver")
    for i in range(led_segs):
        color = "#" + "%06x" % random.randint(0, 0xFFFFFF)
        label = tk.Label(text="", fg="white", bg=color, width=12, height=5)
        label.grid(row = 0, column = i)
    window.after(100, update_window)
    window.mainloop()


# TODO: Figure out how to kill the thread since it will be ether in while True or waiting connections
def on_close():
    #recv_thread.stop()
    sys.exit()

class recvThread(threading.Thread):
    def __init__(self):
        super(recvThread,self).__init__()
        self.connected = False

    def mainrcv(self):
        print("Waiting for connections")
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.bind((HOST, PORT))
        sock.listen()
        conn, addr = sock.accept()
        
        rcv_data_timestamp = 0
        try:
            with conn:
                print('Connected by', addr)
                self.connected = True
                while True:
                    sock.listen()
                    data = conn.recv(1024)
                    if data:
                        decoded = self.decode(data)
                        decoded = [decoded[i:i+3] for i in range(0,len(decoded),3)]
                        #if time.time() - rcv_data_timestamp > 0.016: #Dont always update the variable to not make this blocking
                        global CURRENT_COLORS
                        CURRENT_COLORS = decoded
                        rcv_data_timestamp = time.time()
                        #print(decoded)
        except Exception as e:
            print("[recvThread] Error:", e)
            print("Starting again after 1s")
            self.connected = False
            sock.close()
            time.sleep(1)
            self.mainrcv()
                
    def decode(self, data):
        res = []
        for d in data:
            res.append(d)
        return(res)

    def run(self):
        self.mainrcv()

    def stop(self):
        sys.exit()

    

recv_thread = recvThread()
window = tk.Tk()
window.protocol("WM_DELETE_WINDOW", on_close)
if __name__ == "__main__":
    if len(sys.argv) == 1:
        print("Usage: python dummy_receiver.py [n led segments]")
    else:
        NUM_SEGS = int(sys.argv[1])
        CURRENT_COLORS = [["0", "0", "0"]] * NUM_SEGS
        print("init colors:", CURRENT_COLORS)
        recv_thread.start()
        main(NUM_SEGS)