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

    def get_frm(self):
        return self.frm

    def _create_all(self):
        pass


