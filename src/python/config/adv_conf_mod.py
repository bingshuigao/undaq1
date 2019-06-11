#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type


# this class is the advanced configuration for the vme modules

import tkinter as tk
from adv_conf import adv_conf 

class adv_conf_mod(adv_conf):
    def __init__(self):
        self.var_lst = []

        # if it is the trigger module
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'is_trig_mod',
                 'wid_type' : 'comb',
                 'wid_values' : ['No', 'Yes', 'default'],
                 'comment' : None
                 })
        # clock frequency (in Hz)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'clk_fre',
                 'wid_type' : 'entry',
                 'comment' : None
                 })


########## TEST ##########
#win = tk.Tk()
#adv_conf = adv_conf_fe()
#tk.Button(win, text='button', command=adv_conf.show_win).pack()
#win.mainloop()
