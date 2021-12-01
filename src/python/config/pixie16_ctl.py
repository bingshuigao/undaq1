#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the pixie16 controller. This object manages the configuration data of
# a (virtual) pixie16 controller module
###########3
# By B.Gao Nov. 2021

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class pixie16_ctl(vme_mod):
    def __init__(self, name, mod = None):
        # we need to create the reg_map variable
        self.reg_map = []
        # the ctrl_bit_set/clr are masks of bits to be set/cleared in the
        # control register. We use this two masks in order to avoid
        # directly editing the control registr, instead, we encourage user
        # to tell the frontend which bits they want to set or clear,
        # leaving others in the default values. To do this, the frontend
        # will first read the value of the control register, then do
        # bit-wise operations on the value, then write it back to the
        # register. This is usually necessary because most of the time
        # users want to edit only a few bits of the control register while
        # leaving other bits unchanged. The addresses of the two masks are
        # defined as offset = 500,501
        self.reg_map.append({
                 'off' : 0,
                 'value' : 'default',
                 'name' : 'max evt sz(word)',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        self.reg_map.append({
                 'off' : 1,
                 'value' : 'default',
                 'name' : 'fifo_thresh',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        self.reg_map.append({
                 'off' : 2,
                 'value' : 'default',
                 'name' : 'reset_clk',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        super()._base_init(name, mod)
   

    # get the value of the register in hexdecimal format
    def _get_val_hex(self, reg):
        val = reg['value']
        if val == 'default':
            return 'default'
        return '0x%04x' % val



# test ##########
#win = tk.Tk()
#v1190 = v2718('v2718')
#tk.Button(win, text='button', command=v1190.show_win).pack()
#win.mainloop()
