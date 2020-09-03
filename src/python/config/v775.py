#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the v775 class. This object manages the configuration data of a
# v775 module
###########3
# By B.Gao Oct. 2019

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class v775(vme_mod):
    def __init__(self, name, mod=None):
        # we need to create the reg_map variable
        self.reg_map = []

        # geo
        self.reg_map.append(
                {'off' : 0x1002,
                 'value' : 1,
                 'name' : 'geo',
                 'nbit' : 5,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # ctrl reg
        self.reg_map.append(
                {'off' : 0x1010,
                 'value' : 0x20,
                 'name' : 'ctrl reg1',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # cblt address
        self.reg_map.append(
                {'off' : 0x1004,
                 'value' : 'default',
                 'name' : 'cblt addr',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # cblt ctrl 
        self.reg_map.append(
                {'off' : 0x101a,
                 'value' : 'default',
                 'name' : 'cblt ctl',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['disable', 'last', 'first', 'middle', 'default']
                 })
        # fast clear window 
        self.reg_map.append(
                {'off' : 0x102e,
                 'value' : 'default',
                 'name' : 'fast clr win',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # control bit set2
        self.reg_map.append(
                {'off' : 0x1032,
                 'value' : 0x500,
                 'name' : 'ctrl bit set2',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # control bit clr2
        self.reg_map.append(
                {'off' : 0x1034,
                 'value' : 'default',
                 'name' : 'ctrl bit clr2',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # full scale range
        self.reg_map.append(
                {'off' : 0x1060,
                 'value' : 100,
                 'name' : 'FSR',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # slide constant
        self.reg_map.append(
                {'off' : 0x106a,
                 'value' : 'default',
                 'name' : 'slide const',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # threshold and kill opt
        for i in range(32):
            self.reg_map.append(
                    {'off' : 0x1080+i*2,
                     'value' : 100,
                     'name' : 'threshold %02d' % i,
                     'nbit' : 9,
                     'has_set_wid' : True,
                     'set_wid_type' : 'entry', 
                     'get_set_wid_val' : self._get_val_hex,
                     })

        # This method must be called at the **end**.
        self._base_init(name, mod)
    
    # get the value of the register in hexdecimal format
    def _get_val_hex(self, reg):
        val = reg['value']
        if val == 'default':
            return 'default'
        return '0x%04x' % val

# test ##########
#win = tk.Tk()
#adc32 = madc32('adc32')
#tk.Button(win, text='button', command=adc32.show_win).pack()
#win.mainloop()
