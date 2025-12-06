import paramiko
import sys
import signal
import time
import threading

class raspiConf:
    def __init__(self, ip, listen_port, leds):
        self.ip = ip
        self.port = listen_port
        self.lednums = leds

    def __str__(self):
        return f"{self.ip}:{self.port}"

RASPI_HOSTS = [raspiConf("192.168.1.98", 65431, "-300,480"), raspiConf("192.168.1.99", 65432, "-288,185")]
channels = []
stop_flag = False

def quit_handler(sig, frame):
    print("quit_handler triggered!")
    global channels
    for channel in channels:
        try:
            print(f"Sending CTRL+C to {channel}")
            channel[1].send("\x03")  # ==Ctrl+C
        except Exception as e:
            print(f"Sending ctrl+c to raspi failed {e}")
    sys.exit(0)


def monitor_stdin():
    global stop_flag
    for line in sys.stdin:
        if line.strip().lower() == "stop":
            print("triggering quit handler (via stdin stop command)")
            stop_flag = True
            break


def main():
    global channels
    signal.signal(signal.SIGINT, quit_handler)

    for host in RASPI_HOSTS:
        print(f"Starting receiver on {str(host)}")

        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(host.ip, username="pi", password="raspberry")

        command = f"sudo ./led_server port={host.port} strips={host.lednums}"
        channel = client.invoke_shell()
        channel.settimeout(0)
        channel.send(command + "\n")
        channels.append((host, channel))

    threading.Thread(target=monitor_stdin, daemon=True).start()

    while not stop_flag:
        for host, channel in channels:
            if channel.recv_ready():
                data = channel.recv(1024).decode("utf-8")
                sys.stdout.write(f"{str(host)}: {data}")
                sys.stdout.flush()
                #if "password for" in data:
                #    print(f"Sending password to {str(host)}")
                #    channel.send("raspberry\n") #send password if sudo command is asking for it
            if channel.exit_status_ready():
                print(f"{str(host)} HAS UNEXPECTEDLY QUIT!")
        time.sleep(0.1) # throttle a little bit to prevent high CPU usage

    quit_handler(None, None)


if __name__ == "__main__":
    main()