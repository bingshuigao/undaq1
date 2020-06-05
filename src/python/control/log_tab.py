#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# this is the log tab of the GUI controller 
##########
# By B.Gao Feb. 2019

import tkinter as tk
from tkinter import ttk
from tkinter import messagebox


class log_tab:
    def __init__(self, parent):
       # main frame (to be added as a tab in a notebook widget)
        self.frm = tk.Frame(parent)
        # create all sub windows inside the main frame
        self._create_all()
        # highlight tags
        hl_tag = 0

    def get_frm(self):
        return self.frm

    def _create_all(self):
        # create the log text box
        text_frm = tk.Frame(self.frm, bg='#00ffff', borderwidth=1,
                relief='sunken')
        scrollbar = tk.Scrollbar(text_frm)
        self.text = tk.Text(text_frm, width=10, height=10, wrap='word',
                yscrollcommand=scrollbar.set, borderwidth=0,
                highlightthickness=0)
        scrollbar.config(command=self.text.yview)
        scrollbar.pack(side='right', fill='y')
        self.text.pack(side='left', fill='both', expand=True)
        text_frm.place(x=0, y=0, width=800, height=525)
        self.text.insert(tk.END, 'begin logging...\n')
        self.text.config(state=tk.DISABLED)

    def insert_log(self, txt, hl=False):
        self.text.insert(tk.END, datetime.datetime.now().ctime()+txt+'\n')
        if hl:
            hl_tag += 1
            self.text.tag_add(str(hl_tag), str(tk.END)+'.0', str(tk.END)+'.100')
            self.text.tag_config(str(hl_tag), foreground='red')



