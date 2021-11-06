import socket
from sys import byteorder

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
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
            if data:
                decoded = decode(data)
                print(decoded)
                #DO LED STUFF

def decode(data):
    res = []
    for d in data:
        res.append(d)
    return(res)
    


func()