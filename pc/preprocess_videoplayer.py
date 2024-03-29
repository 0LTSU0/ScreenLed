
import os
import time
import tkinter as tk
import pathlib
import socket
import json
import pickle

os.add_dll_directory(r'C:\Program Files\VideoLAN\VLC')
import vlc


class videoPlayer():
    def __init__(self):
        self.player = vlc.MediaPlayer()

    def set_file(self, file):
        self.player.set_media(vlc.Media(file))

    def play(self):
        self.player.play()

    def pause(self):
        self.player.pause()

    def get_timestamp(self):
        return self.player.get_time()

    def get_media_length(self):
        return self.player.get_length()

    def set_media_time(self, timestamp):
        if timestamp != 0:
            self.player.set_time(timestamp)

    def volumeup(self):
        current_volume = self.player.audio_get_volume()
        if current_volume < 100:
            self.player.audio_set_volume(current_volume + 10)
        print("Current volume:", current_volume)

    def volumedown(self):
        current_volume = self.player.audio_get_volume()
        if current_volume > 0:
            self.player.audio_set_volume(current_volume - 10)
        print("Current volume:", current_volume)

    def mute(self):
        self.player.audio_set_mute(not self.player.audio_get_mute())


class ctrlWindow(tk.Tk):
    def __init__(self, file, connect=False, conf=None):
        self.ctrl_window = tk.Tk()
        self.ctrl_window.title("ScreenLed Player")
        self.play = tk.Button(self.ctrl_window, text = "Pause", command=lambda: [vlcplayer.pause(), self.handle_pausing("pause")])
        self.play.grid(row = 0, column = 0)
        self.pause = tk.Button(self.ctrl_window, text = "Play", command=lambda: [vlcplayer.play(), self.handle_pausing("play")])
        self.pause.grid(row = 0, column = 1)
        self.timebar = tk.Scale(self.ctrl_window, from_=0, to=vlcplayer.get_media_length(), orient=tk.HORIZONTAL, length=450)
        self.timebar.bind("<ButtonRelease-1>", self.set_video_time)
        self.timebar.grid(row = 0, column = 2)
        self.volumeup = tk.Button(self.ctrl_window, text = "Volume +", command=vlcplayer.volumeup)
        self.volumeup.grid(row = 0, column = 4)
        self.volumedown = tk.Button(self.ctrl_window, text = "Volume -", command=vlcplayer.volumedown)
        self.volumedown.grid(row = 0, column = 3)
        self.mute = tk.Button(self.ctrl_window, text = "Mute", command=vlcplayer.mute)
        self.mute.grid(row = 0, column = 5)

        #self.playback_timestamp = 0
        self.prev_timestamp = time.time()
        self.calc_timestamp = 0
        self.update_counter = 0
        self.is_paused = False

        self.connection_enabled = connect
        self.socket = None
        if self.connection_enabled and conf:
            self.connect_socket(conf)
        elif connect and not conf:
            print("Missing config for connection!")

        self.file = file
        self.rgbdata = self.load_rgb_data(file)
        self.current_rgb_data_timestamp = int(next(iter(self.rgbdata)))
        self.previous_rgb_data_ts = self.current_rgb_data_timestamp
        self.prev_rgb_sent_abstime = time.time()

        self.ctrl_window.after(0, self.send_preprosessed_rgb)
        self.ctrl_window.after(1000, self.update_timebar())

        self.ctrl_window.mainloop()


    def load_rgb_data(self, file):
        #Expect that .ppv is in same folder with same filename as video
        with open(file.parent.absolute().joinpath(file.stem + ".ppv"), "rb") as f:
            rgb_data = pickle.load(f)
            print(".ppv loaded succesfully")
        return rgb_data


    def connect_socket(self, conf):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((conf.get("address"), conf.get("port")))
        print("Connected succefully")


    def handle_pausing(self, button_name):
        print("handle_pausing")
        if button_name == "play":
            self.is_paused = False
        elif button_name == "pause":
            self.is_paused = not self.is_paused

        #Sync times on unpause
        if not self.is_paused:
            self.calc_timestamp = vlcplayer.get_timestamp()
            self.prev_timestamp = time.time()


    def set_video_time(self, *args, **kwargs):
        new_time = self.timebar.get()
        #reload all rgbdata if skipped back
        if new_time < self.calc_timestamp:
            self.rgbdata = self.load_rgb_data(self.file)
            self.current_rgb_data_timestamp = int(next(iter(self.rgbdata)))
            self.previous_rgb_data_ts = self.current_rgb_data_timestamp
        vlcplayer.set_media_time(new_time)
        self.calc_timestamp = new_time


    def update_timebar(self):
        self.timebar.set(vlcplayer.get_timestamp())
        
        #Check time sync about every 5s
        if self.update_counter % 5 == 0:
            print("calctime - vlctime before sync", self.calc_timestamp - vlcplayer.get_timestamp())
            self.calc_timestamp = vlcplayer.get_timestamp()
        
        self.update_counter += 1
        self.ctrl_window.after(1000, self.update_timebar)


    def send_preprosessed_rgb(self):
        if not self.is_paused:
            curr_absolute_time = time.time()
            self.calc_timestamp += (curr_absolute_time - self.prev_timestamp) * 1000
            self.prev_timestamp = curr_absolute_time
            #print("clac time", self.calc_timestamp, "vlc time", vlcplayer.get_timestamp())
            while self.calc_timestamp > self.current_rgb_data_timestamp:
                del self.rgbdata[str(self.current_rgb_data_timestamp)]
                self.current_rgb_data_timestamp = int(next(iter(self.rgbdata))) #fisrt key after deletion
            current_rgb_data = self.rgbdata[str(self.current_rgb_data_timestamp)]
            #print("sending data: ", self.current_rgb_data_timestamp, current_rgb_data)
            self.send_rgb_data(current_rgb_data)

        self.ctrl_window.after(10, self.send_preprosessed_rgb)


    def send_rgb_data(self, data):
        if self.previous_rgb_data_ts != self.current_rgb_data_timestamp:
            packet = bytes(0)
            data.reverse()
            for i in range(len(data)):
                x = int(data[i][2]).to_bytes(1, 'big')
                y = int(data[i][1]).to_bytes(1, 'big')
                z = int(data[i][0]).to_bytes(1, 'big')
                appendstr = x + y + z
                packet = packet + appendstr
            if self.connection_enabled:
                self.socket.sendall(packet)
            #print("fps: ", 1 / (time.time() - self.prev_rgb_sent_abstime))
            #print("\r" + "fps: " + str(1 / (time.time() - self.prev_rgb_sent_abstime)), end="")
            print("\r" + "fps: " + str(1 / (time.time() - self.prev_rgb_sent_abstime)), end=" | ")
            self.prev_rgb_sent_abstime = time.time()

        self.previous_rgb_data_ts = self.current_rgb_data_timestamp



if __name__ == "__main__":
    video_path = pathlib.Path(r"D:\Tiedostot\Live music\babymetal\BABYMETAL -「Elevator Girl (Japanese Ver.)」[Live Compilation] [字幕 _ SUBTITLED] [HQ].mp4")
    #video_path = pathlib.Path(r"D:\Tiedostot\ledtestvid.mp4")
    
    vlcplayer = videoPlayer()
    vlcplayer.set_file(video_path)
    vlcplayer.play()

    conf_path = pathlib.Path(__file__)
    conf_path = conf_path.parent.parent.absolute().joinpath("conf.json")
    with open(conf_path, "r") as f:
        conf=json.load(f)
    
    ctrl_app = ctrlWindow(video_path, connect=True, conf=conf)
