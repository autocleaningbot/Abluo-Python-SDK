#!/usr/bin/env python3
import threading
import sys
import datetime
import tkinter as tk
from tkinter import ttk
import tkinter.scrolledtext as st
sys.path.append("..")
from logitech_controller.controller_gui import AbluoController


def rosPlaceHolder():
    print("Welcome to ROS")


class thread_with_trace(threading.Thread):
  def __init__(self, *args, **keywords):
    threading.Thread.__init__(self, *args, **keywords)
    self.killed = False
 
  def start(self):
    self.__run_backup = self.run
    self.run = self.__run     
    threading.Thread.start(self)
 
  def __run(self):
    sys.settrace(self.globaltrace)
    self.__run_backup()
    self.run = self.__run_backup
 
  def globaltrace(self, frame, event, arg):
    if event == 'call':
      return self.localtrace
    else:
      return None
 
  def localtrace(self, frame, event, arg):
    if self.killed:
      if event == 'line':
        raise KeyboardInterrupt
    return self.localtrace
 
  def kill(self):
    self.killed = True


class TextRedirector(object):
    def __init__(self, widget, tag="stdout"):
        self.widget = widget
        self.tag = tag

    def write(self, str):
        if str == '\n':
            pass
        else:
            self.widget.configure(state="normal")
            now = datetime.datetime.now()
            timestamp = "[%02d:%02d:%02d]: " % (now.hour, now.minute, now.second)
            printstr = timestamp + str + "\n"
            self.widget.insert(tk.END, printstr, (self.tag,))
            self.widget.see(tk.END)
            self.widget.configure(state="disabled")

    def flush(self):
        pass


class tkinterApp(tk.Tk):
    def __init__(self, *args, **kwargs):
        tk.Tk.__init__(self, *args, **kwargs)
        container = tk.Frame(self)
        container.pack(fill="both", expand=True)

        container.grid_rowconfigure(0, weight = 1)
        container.grid_columnconfigure(0, weight = 1)

        self.frames = {}
        self.controllerThread = None
        self.rosThread = None

        for F in (StartPage, Controller, ROS):
            frame = F(container, self)
            frame.grid(row=0, column=0, sticky="nsew")
            self.frames[F] = frame
        
        self.show_frame(StartPage)

    def show_frame(self, cont):
        frame = self.frames[cont]
        
        if cont == StartPage:
            if self.controllerThread != None:
                print("Exiting Controller Mode")
                print("-------------------------------------------------")
                self.controllerThread.kill()
                self.controllerThread.join()
                self.controllerThread = None
                frame.tkraise()
            elif self.rosThread != None:
                print("Exiting ROS Mode")
                print("-------------------------------------------------")
                self.rosThread.kill()
                self.rosThread.join()
                self.rosThread = None
                frame.tkraise()
            else:
                frame.tkraise()
        elif cont == Controller:
            frame.tkraise()
            sys.stdout = TextRedirector(frame.text_area, "stdout")
            controller = AbluoController(8, 0x60, 0x70, 0x65)
            self.controllerThread = thread_with_trace(target=controller.main, daemon=True)
            self.controllerThread.start()
        elif cont == ROS:
            frame.tkraise()
            sys.stdout = TextRedirector(frame.text_area, "stdout")
            self.rosThread = thread_with_trace(target=rosPlaceHolder, daemon=True)
            self.rosThread.start()


class StartPage(tk.Frame):
    def __init__(self, parent, mainWin):
        tk.Frame.__init__(self, parent)

        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(1, weight=1)

        label = ttk.Label(self, text="Welcome to Abluo", font=("Times New Roman", 25))
        label.grid(row=0, column=0, columnspan=2, sticky="N", padx=20, pady=50)

        controller_button = tk.Button(self, text="Controller", command=lambda:mainWin.show_frame(Controller), height=50, width=120)
        controller_button.grid(row=1, column=0, sticky="E", padx=30, pady=100)

        ros_button = tk.Button(self, text="ROS", command=lambda:mainWin.show_frame(ROS), height=50, width=120)
        ros_button.grid(row=1, column=1, sticky="W", padx=30, pady=100)


class Controller(tk.Frame):
    def __init__(self, parent, mainWin):
        tk.Frame.__init__(self, parent)

        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(2, weight=1)

        label = ttk.Label(self, text="You are now using Controller Mode", font=("Times New Roman", 15))
        label.grid(row=0, column=0, sticky="N", padx=20, pady=20)

        exit_button = ttk.Button(self, text="Exit", command=lambda:mainWin.show_frame(StartPage), padding=5)
        exit_button.grid(row=1, column=0, padx=20, pady=20)

        self.text_area = st.ScrolledText(self)
        self.text_area.grid(row=2, column=0)
        self.text_area.tag_configure("stderr", foreground="#b22222")
        self.text_area.see(tk.END)
        self.text_area.configure(state='disabled')


class ROS(tk.Frame):
    def __init__(self, parent, mainWin):
        tk.Frame.__init__(self, parent)

        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(2, weight=1)

        label = ttk.Label(self, text="You are now using Autonomous Mode", font=("Times New Roman", 15))
        label.grid(row=0, column=0, sticky="N", padx=20, pady=20)

        exit_button = ttk.Button(self, text="Exit", command=lambda:mainWin.show_frame(StartPage), padding=5)
        exit_button.grid(row=1, column=0, padx=20, pady=20)

        self.text_area = st.ScrolledText(self)
        self.text_area.grid(row=2, column=0)
        self.text_area.tag_configure("stderr", foreground="#b22222")
        self.text_area.see(tk.END)
        self.text_area.configure(state='disabled')


if __name__ == "__main__":
    app = tkinterApp()
    app.title("Abluo Command Interface")
    app.state("zoomed")
    # app.attributes('-fullscreen', True)
    app.mainloop()
