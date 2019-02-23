#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type


# this class is the advanced configuration for the GUI controler.

import tkinter as tk
from adv_conf import adv_conf 

class adv_conf_ctl(adv_conf):
    def __init__(self):
        self.var_lst = []

        # the listening port
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'port',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the update interval (ms) 
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 't_ms',
                 'wid_type' : 'entry',
                 'comment' : None
                 })



########## TEST ##########
#win = tk.Tk()
#adv_conf = adv_conf_fe()
#tk.Button(win, text='button', command=adv_conf.show_win).pack()
#win.mainloop()
