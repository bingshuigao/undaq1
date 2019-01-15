#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# this is the analyzer tab of the notebook widget in the main window. It
# manages everything (settings) of the analyzer.
##########
# By B.Gao Jan. 2019

import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
from adv_conf_ana import adv_conf_ana


class ana:
    def __init__(self, parent):
        # the advanced settings
        self.adv_conf = adv_conf_ana()
       
       # main frame (to be added as a tab in a notebook widget)
        self.frm = tk.Frame(parent)
        # create all sub windows inside the main frame
        self._create_all()

    def get_frm(self):
        return self.frm

    def get_adv_conf(self):
        return self.adv_conf

    def _create_all(self):
        self.butt_adv = tk.Button(self.frm, text='advanced...',
                command=self._adv_conf)
        self.butt_adv.place(x =560, y=400, width=100, height=25)

    def _adv_conf(self):
        self.adv_conf.show_win()
        self.adv_conf.get_win().grab_set()
        self.frm.wait_window(self.adv_conf.get_win())

    def _show_msg(self, msg, flag='info'):
        if flag == 'err':
            messagebox.showerror('error', msg, parent=self.frm)
        else:
            messagebox.showinfo('info', msg, parent=self.frm)

            

# tests ##########################
#win = tk.Tk()
#win.geometry('800x650')
#frm = tk.Frame(win)
#frm.place(x=0, y=30, width=800, height=600) 
#tab_ctrl = ttk.Notebook(frm)
#tab1 = ebd(tab_ctrl)
#tab_ctrl.add(tab1.get_frm(), text='frontend')
#tab_ctrl.pack(fill=tk.BOTH, expand=1) 
#win.mainloop()
