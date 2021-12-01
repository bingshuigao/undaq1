#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type


# this class is the advanced configuration for the pixie16 contoller modules

import tkinter as tk
from adv_conf import adv_conf 

class adv_conf_pixie_ctl(adv_conf):
    def __init__(self):
        self.var_lst = []

        # if it is the trigger module
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'pixie16_ctl1',
                 'wid_type' : 'comb',
                 'wid_values' : ['No', 'Yes', 'default'],
                 'comment' : None
                 })
        # clock frequency (in Hz)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'pixie16_ctl2',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # clock offset (in clock ticks)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'pixie16_ctl3',
                 'wid_type' : 'entry',
                 'comment' : None
                 })


########## TEST ##########
#win = tk.Tk()
#adv_conf = adv_conf_fe()
#tk.Button(win, text='button', command=adv_conf.show_win).pack()
#win.mainloop()
