#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# this is the ring buffers tab of the GUI controller 
##########
# By B.Gao Feb. 2019

import tkinter as tk
from tkinter import ttk
from tkinter import messagebox


class rb_tab:
    def __init__(self, parent):
       # main frame (to be added as a tab in a notebook widget)
        self.frm = tk.Frame(parent)
        # create all sub windows inside the main frame
        self._create_all()

        self.if_show = False
        self.rb_width = 100
        self.rb_height = 100

    def get_frm(self):
        return self.frm

    def _create_all(self):
        # we need to create scroll bar, similar as config.py
        self.canv = tk.Canvas(self.frm, bg='gray')
        self.rb_frm = tk.Frame(self.canv, bg='gray') 
        self.vsb = tk.Scrollbar(self.frm, orient='vertical',
                        command=self.canv.yview)
        self.canv.configure(yscrollcommand=self.vsb.set)
        self.vsb.place(x=770, y=30, width=20, height=500)
        self.canv.place(x=0, y=30, width=790, height=500)
        self.canv.create_window((0,0), window=self.rb_frm, anchor='nw')
        self.rb_frm.bind('<Configure>', self._frm_conf)
        self.hsb = tk.Scrollbar(self.frm, orient='horizontal',
                        command=self.canv.xview)
        self.canv.configure(xscrollcommand=self.hsb.set)
        self.hsb.place(x=0, y=530, width=790, height=20)
        self.rb_frm.bind('<Configure>', self._frm_conf)

        # create a button to show/hide the ring buffer map
        self.butt_show_rb = tk.Button(self.frm, text='show/hide ring buffer map', command=self._show_hide)
        self.butt_show_rb.place(x=0,y=0)

    def _frm_conf(self, evt):
        self.canv.configure(scrollregion=self.canv.bbox('all'))

    def _show_hide(self):
        self.if_show = not self.if_show

    def create_canv(self, n_fe, n_mod):
        # The width and hight of the canvas depends on the number of frontend
        # and vme modules
        self.sub_canv_width = n_fe * self.rb_width
        if (n_mod+1) > n_fe:
            self.sub_canv_width = (n_mod+1) * self.rb_width
        self.sub_canv_height = self.rb_height * 5 + 8 * self.rb_height / 2
        self.sub_canv = tk.Canvas(self.rb_frm, width=self.sub_canv_width,
                height=self.sub_canv_height)
        self.sub_canv.pack()





