#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the v977. This object manages the configuration data of a v977 module
########
# By B.Gao Sep. 2018

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class v977(vme_mod):
    def __init__(self, name, mod = None):
        # we need to create the reg_map variable
        self.reg_map = []

        # Input set
        self.reg_map.append(
                {'off' : 0x0,
                 'value' : 'default',
                 'name' : 'input set',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # input mask
        self.reg_map.append(
                {'off' : 0x2,
                 'value' : 'default',
                 'name' : 'input mask',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # Output set
        self.reg_map.append(
                {'off' : 0xa,
                 'value' : 'default',
                 'name' : 'ouput set',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # ouput mask
        self.reg_map.append(
                {'off' : 0xc,
                 'value' : 'default',
                 'name' : 'output mask',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        
        # irq mask
        self.reg_map.append(
                {'off' : 0xe,
                 'value' : 'default',
                 'name' : 'irq mask',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # test control register bit set, for the meanings of the set/clr
        # registers, reffer to the v1190 module.
        self.reg_map.append(
                {'off' : 500,
                 'value' : 'default',
                 'name' : 'test inp set',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # test control register bit clear
        self.reg_map.append(
                {'off' : 501,
                 'value' : 'default',
                 'name' : 'test inp clr',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # interrupt level
        self.reg_map.append({
                 'off' : 0x20,
                 'value' : 'default',
                 'name' : 'int lev',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['0','1','2','3','4','5','6','7','default']
                 })
        # interrupt vector
        self.reg_map.append({
                 'off' : 0x22,
                 'value' : 'default',
                 'name' : 'int vec',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # control register bit set, for the meanings of the set/clr
        # registers, reffer to the v1190 module.
        self.reg_map.append(
                {'off' : 502,
                 'value' : 'default',
                 'name' : 'ctrl set',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # control register bit clear
        self.reg_map.append(
                {'off' : 503,
                 'value' : 'default',
                 'name' : 'ctrl clr',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
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
#v1190 = v830('v830')
#tk.Button(win, text='button', command=v1190.show_win).pack()
#win.mainloop()
