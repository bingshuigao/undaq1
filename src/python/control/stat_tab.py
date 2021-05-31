#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# this is the status tab of the GUI controller 
##########
# By B.Gao Feb. 2019

import tkinter as tk
from tkinter import ttk
from tkinter import messagebox


class stat_tab:
    def __init__(self, parent):
       # main frame (to be added as a tab in a notebook widget)
        self.frm = tk.Frame(parent)
        # create all sub windows inside the main frame
        self._create_all()
        
        self.cur_t = 0
        self.cur_cnt = 0

    def get_frm(self):
        return self.frm

    def _create_all(self):
        self.label_rate = tk.Label(self.frm, text='data rate:', height=1,
                width=100, anchor='w')
        self.label_rate.place(x=0, y=0)
        self.label_data_sz = tk.Label(self.frm, text='data size:', height=1,
                width=100, anchor='w')
        self.label_data_sz.place(x=0, y=25)

    def set_rate(self, rate):
        self.label_rate.config(text='data rate: %.2f kB/s'%rate)

    def set_data_sz(self, sz):
        self.label_data_sz.config(text='data size: %.3f MB'%sz)

    def update_evt_cnt(self, msg):
        ts_hi     = int.from_bytes(msg[0:4], 'little')
        ts_lo     = int.from_bytes(msg[4:8], 'little')
        n_module  = int.from_bytes(msg[8:12],'little')
        ts = (ts_hi<<32) + ts_lo
        delta_t = ts - self.cur_t
        for i in range(n_module):
            int_entry = int.from_bytes(msg[12+(i//2):16+(i//2)], 'little')
            slot = (int_entry >>((i%2)*16)) & 0xff
            crate = ((int_entry >> ((i%2)*16) + 8)) & 0xff
            cnt = int.from_bytes(msg[53*4+i:54*4+i], 'little')
            delta_cnt = cnt - self.cur_cnt
            rate = delta_cnt*1000./delta_t
            print('slot: %02d, crate: %02d, cnt: %-10d, rate: %10.3f/s' % (slot, crate, cnt, rate))
        self.cur_t = ts
        self.cur_cnt = cnt




