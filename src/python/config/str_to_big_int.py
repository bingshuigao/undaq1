#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type


# This is a help script which converts a string to a big integer (see comments
# in adv_conf_fe.py)

import tkinter as tk
from subprocess import Popen, PIPE

def convert():
    s = str_box.get()
    big_int = 0
    for (i,x) in enumerate(s):
        big_int += ord(x) << (i*8)
    int_box.delete(0, tk.END)
    int_box.insert(0, str(big_int))

def cp():
    root_win.clipboard_clear()
    root_win.clipboard_append(int_box.get())
    int_box.selection_range(0, tk.END)



root_win = tk.Tk()
root_win.geometry('500x200')
str_box = tk.Entry(root_win)
str_box.place(x=0, y=0, width=400, height=50)
int_box = tk.Entry(root_win)
int_box.place(x=0, y=60, width=400, height=50)
butt = tk.Button(root_win, text='convert', command=convert)
butt.place(x=0, y=120, width=150, height=50)
copy_to_cb = tk.Button(root_win, text='copy to clipboard', command=cp)
copy_to_cb.place(x=210, y=120, width=150, height=50)
root_win.mainloop()



