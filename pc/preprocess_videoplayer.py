
import os
import time
os.add_dll_directory(r'C:\Program Files\VideoLAN\VLC')
import vlc
import tkinter as tk


class videoPlayer():
    def __init__(self):
        #self.player = vlc.Instance("--loop")
        self.player = vlc.MediaPlayer()

    def set_file(self, file):
        #self.medialist = self.player.media_list_new()
        #self.medialist.add_media(self.player.media_new(file))
        #self.lplayer = self.player.media_list_player_new()
        #self.lplayer.set_media_list(self.medialist)
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




class ctrlWindow(tk.Tk):
    def __init__(self):
        self.ctrl_window = tk.Tk()
        self.play = tk.Button(self.ctrl_window, text = "Pause", command=vlcplayer.pause)
        self.play.grid(row = 0, column = 0)
        self.pause = tk.Button(self.ctrl_window, text = "Play", command=vlcplayer.play)
        self.pause.grid(row = 0, column = 1)
        self.timebar = tk.Scale(self.ctrl_window, from_=0, to=vlcplayer.get_media_length(), orient=tk.HORIZONTAL, length=400)
        self.timebar.bind("<ButtonRelease-1>", self.set_video_time)
        self.timebar.grid(row = 0, column = 2)

        #self.playback_timestamp = 0
        self.prev_timestamp = time.time()
        self.calc_timestamp = 0

        self.ctrl_window.after(0, self.send_preprosessed_rgb)
        self.ctrl_window.after(1000, self.update_timebar())
        self.ctrl_window.after

        self.ctrl_window.mainloop()

    def set_video_time(self, *args, **kwargs):
        vlcplayer.set_media_time(self.timebar.get())

    def update_timebar(self):
        self.timebar.set(vlcplayer.get_timestamp())
        self.calc_timestamp = vlcplayer.get_timestamp()
        #if self.start_timestamp == 0:
        #    self.start_timestamp = vlcplayer.get_timestamp / 1000
        self.ctrl_window.after(1000, self.update_timebar)

    def send_preprosessed_rgb(self):
        self.calc_timestamp += (time.time() - self.prev_timestamp) * 1000
        self.prev_timestamp = time.time()
        print("clac time", self.calc_timestamp, "vlc time", vlcplayer.get_timestamp())
        self.ctrl_window.after(10, self.send_preprosessed_rgb)

if __name__ == "__main__":
    vlcplayer = videoPlayer()
    vlcplayer.set_file(r"D:\Tiedostot\Live music\babymetal\BABYMETAL -「Elevator Girl (Japanese Ver.)」[Live Compilation] [字幕 _ SUBTITLED] [HQ].mp4")
    vlcplayer.play()

    ctrl_app = ctrlWindow()
