
from turtle import width
import kivy
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.textinput import TextInput
from kivy.uix.dropdown import DropDown
from kivy.uix.checkbox import CheckBox
from kivy.clock import Clock
from kivy.base import runTouchApp
import json
import sys

import os
import subprocess
from pc import mainpc


class screenLed_gui(App):

    def preview_chbox(self, checkbox, value):
        if value:
            print('The preview checkbox is active')
        else:
            print('The preview checkbox is inactive')

    def ssfunc(self, event):
        run = True
        if "start" in self.ssbutton.text:
            print("startstuff")
            #construct json config
            conf = {}
            conf["address"] = self.ip.text
            try:
                conf["port"] = int(self.port.text)
            except Exception:
                run = False
            conf["resolution"] = self.main_reso_btn.text
            conf["enable_preview"] = self.preview.active
            conf["enable_shitpc"] = self.shitpcmode.active
            conf["python"] = self.python_path.text
            conf["target_fps"] = self.target_fps.text

            with open("conf.json", "w") as f:
                json.dump(conf, f)

            if run:
                self.ssbutton.text = "Stop"
                self.subproc = run_pcpy(python_interpeter=self.python_path.text)
                self.subproc.start()
        
        else:
            print("stopstuff")
            self.ssbutton.text = "Save config and start"
            self.subproc.stop()
        

    def build(self):
        self.mainwindow = BoxLayout(orientation='vertical')
        #self.mainwindow.cols = 1
        
        self.toplabelview = GridLayout()
        self.toplabelview.rows = 1
        self.toplabelview.cols = 1
        self.settingsview = GridLayout(cols=2, row_force_default=True, row_default_height=30)
        self.mainbtnview = GridLayout(cols=1)
        

        #-----------Header----------
        self.header = Label(text='ScreenLed gui', font_size=32)
        self.toplabelview.add_widget(self.header)

        #-----------Settings---------
        #ip
        self.ip_text = Label(text="Host IP", font_size=16)
        self.settingsview.add_widget(self.ip_text)
        self.ip = TextInput(text="192.168.1.69", multiline=False)
        self.settingsview.add_widget(self.ip)
        

        #port
        self.port_text = Label(text="Host PORT", font_size=16)
        self.settingsview.add_widget(self.port_text)
        self.port = TextInput(text="65432", multiline=False)
        self.settingsview.add_widget(self.port)


        #targetFPS (multithreading only)
        self.target_fps_text = Label(text="Target FPS (multithreaded only)", font_size=16)
        self.settingsview.add_widget(self.target_fps_text)
        self.target_fps = TextInput(text="60", multiline=False)
        self.settingsview.add_widget(self.target_fps)


        #python cmd
        self.python_text = Label(text="Python in path", font_size=16)
        self.settingsview.add_widget(self.python_text)
        self.python_path = TextInput(text="python", multiline=False)
        self.settingsview.add_widget(self.python_path)


        #resolution
        self.resolution_text = Label(text="Resolution", font_size=16)
        self.settingsview.add_widget(self.resolution_text)
        resolution = DropDown()
        
        #Resolutions to be added here
        option1 = Button(text="1920x1080", height=30, size_hint_y=None)
        option1.bind(on_release=lambda option1: resolution.select(option1.text))
        resolution.add_widget(option1)
        option2 = Button(text="TBD", height=30, size_hint_y=None)
        option2.bind(on_release=lambda option2: resolution.select(option2.text))
        resolution.add_widget(option2)

        self.main_reso_btn = Button(text="1920x1080")
        self.main_reso_btn.bind(on_release=resolution.open)
        resolution.bind(on_select=lambda instance, x: setattr(self.main_reso_btn, 'text', x))
        self.settingsview.add_widget(self.main_reso_btn)
        

        #preview
        self.preview_text = Label(text="Show capture preview", font_size=16)
        self.settingsview.add_widget(self.preview_text)
        self.preview = CheckBox()
        self.preview.bind(active=self.preview_chbox)
        self.settingsview.add_widget(self.preview)


        #shitpc variants
        self.shitpc_text = Label(text="Low-end PC mode", font_size=16)
        self.settingsview.add_widget(self.shitpc_text)
        self.shitpcmode = CheckBox()
        self.shitpcmode.active = True
        #self.shitpcmode.bind(active=self.preview_chbox)
        self.settingsview.add_widget(self.shitpcmode)


        #startstop button
        #self.statusmsg = Label(text="Status:", font_size=16)
        #self.mainbtnview.add_widget(self.statusmsg)
        self.ssbutton = Button(text="Save config and start", font_size=32)
        self.ssbutton.bind(on_press=self.ssfunc)
        self.mainbtnview.add_widget(self.ssbutton)


        #Combine everything to one widget
        self.mainwindow.add_widget(self.toplabelview)
        self.mainwindow.add_widget(self.settingsview)
        self.mainwindow.add_widget(self.mainbtnview)

        #Update status msg
        #Clock.schedule_interval(self.update_status, 0.5)

        return self.mainwindow


class run_pcpy():
    def __init__(self, python_interpeter="python"):
        self.proc = None
        self.python = python_interpeter
    
    def start(self):
        try:
            if sys.argv[1] == "multith":
                self.proc = subprocess.Popen([self.python, "pc_multithread.py"])
            else:
                self.proc = subprocess.Popen([self.python, "pc.py"])
        except IndexError:
            self.proc = subprocess.Popen([self.python, "pc.py"])

    def stop(self):
        self.proc.kill()
        self.proc.wait()


if __name__ == '__main__':
    print(sys.argv)
    if "pc" in os.listdir():
        print("Changing CWD to correct folder")
        try:
            os.chdir("pc")
        except Exception:
            print("nevermind")
        print(os.listdir())
    
    screenLed_gui().run()