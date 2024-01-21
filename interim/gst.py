import cv2
import os
gstreamer_str = "gst-launch-1.0 videotestsrc ! videoconvert ! autovideosink"
strbld = "gst-launch-1.0"
#os.system(gstreamer_str)
from tkinter import *

def videoTestSrc():
    global strbld
    strbld += " videotestsrc !"

def videoConvert():
    global strbld
    strbld += " videoconvert !"

def autoVideoSink():
    global strbld
    strbld += " autovideosink"

def execute():
    global strbld
    os.system(strbld)

root = Tk()
root.title("Hello")
root.geometry("500x500")

videoTestSrc_button = Button(root, text="videotestsrc", command=videoTestSrc)
videoTestSrc_button.place(x=50,y=50)

videoTestSrc_button = Button(root, text="videoconvert", command=videoConvert)
videoTestSrc_button.place(x=50,y=80)

videoTestSrc_button = Button(root, text="autovideosink", command=autoVideoSink)
videoTestSrc_button.place(x=50,y=110)

videoTestSrc_button = Button(root, text="execute", command=execute)
videoTestSrc_button.place(x=50,y=140)

root.mainloop()