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

    def get_frm(self):
        return self.frm

    def _create_all(self):
        self.label_rate = tk.Label(self.frm, text='data rate:', height=1,
                width=100, anchor='w')
        self.label_rate.place(x=0, y=0)

    def set_rate(self, rate):
        self.label_rate.config(text='data rate: %.2f kB/s'%rate)


