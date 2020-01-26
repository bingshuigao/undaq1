#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the v830. This object manages the configuration data of a v830 module
########
# By B.Gao Sep. 2018

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class v830(vme_mod):
    def __init__(self, name, mod = None):
        # we need to create the reg_map variable
        self.reg_map = []

        # channel enable pattern
        self.reg_map.append({
                 'off' : 0x1100,
                 'value' : 0xffffffff,
                 'name' : 'ch enable',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # Dwell time
        self.reg_map.append({
                 'off' : 0x1104,
                 'value' : 'default',
                 'name' : 'Dwell T',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # control bit set
        self.reg_map.append({
                 'off' : 0x110a,
                 'value' : 0xb1,
                 'name' : 'ctrl bit set',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        
        # control bit clear
        self.reg_map.append({
                 'off' : 0x110c,
                 'value' : 'default',
                 'name' : 'ctrl bit clr',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # GEO
        self.reg_map.append({
                 'off' : 0x1110,
                 'value' : 'default',
                 'name' : 'GEO',
                 'nbit' : 5,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

       
        # cblt address
        self.reg_map.append({
                 'off' : 0x111c,
                 'value' : 'default',
                 'name' : 'cblt addr',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # cblt contrl
        self.reg_map.append({
                 'off' : 0x111e,
                 'value' : 'default',
                 'name' : 'cblt ctrl',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['disable', 'last', 'first', 'middle',
                     'default']
                 })

        # almost full level
        self.reg_map.append({
                 'off' : 0x112c,
                 'value' : 'default',
                 'name' : 'almost full',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # BLT event number
        self.reg_map.append({
                 'off' : 0x1130,
                 'value' : 'default',
                 'name' : 'blt evt N',
                 'nbit' : 8,
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
#v1190 = v830('v830')
#tk.Button(win, text='button', command=v1190.show_win).pack()
#win.mainloop()
