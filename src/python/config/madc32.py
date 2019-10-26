#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the madc32 class. This object manages the configuration data of a
# madc32 module
###########3
# By B.Gao Sep. 2018

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class madc32(vme_mod):
    def __init__(self, name, mod=None):
        # we need to create the reg_map variable
        self.reg_map = []
        # threshold
        self.thre_w = []
        for i in range(32):
            self.reg_map.append(
                    {'off' : 0x4000+i*2,
                     'value' : 'default',
                     'name' : 'threshold %02d' % i,
                     'nbit' : 13,
                     'has_set_wid' : True,
                     'set_wid_type' : 'entry', 
                     })
        # module id
        self.reg_map.append(
                {'off' : 0x6004,
                 'value' : 'default',
                 'name' : 'module id',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # max transfer data
        self.reg_map.append(
                {'off' : 0x601a,
                 'value' : 'default',
                 'name' : 'max trans data',
                 'nbit' : 14,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['unlimited', 'default'],
                 })
        # cblt and mcst contrl:
        self.reg_map.append(
                {'off' : 0x6020,
                 'value' : 'default',
                 'name' : 'cblt&mcst ctrl',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # cblt and mcst address
        self.reg_map.append(
                {'off' : 0x6022,
                 'value' : 'default',
                 'name' : 'cblt addr',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        self.reg_map.append(
                {'off' : 0x6024,
                 'value' : 'default',
                 'name' : 'mcst addr',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # data length format
        self.reg_map.append(
                {'off' : 0x6032,
                 'value' : 'default',
                 'name' : 'data len fmat',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['8bit', '16bit', '32bit', '64bit', 
                                     'default'],
                 })
        # multi event
        self.reg_map.append(
                {'off' : 0x6036,
                 'value' : 'default',
                 'name' : 'multi event',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['0(do not use!)', '1(do not use!)', 
                     '2(do not use!)', '3(pls use!)', 'default'],
                 })
        # marking type
        self.reg_map.append(
                {'off' : 0x6038,
                 'value' : 'default',
                 'name' : 'marking type',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['evt cnt', 'ts', 'unknown', 'ext ts', 
                                     'default'],
                 })
        # adc resolution
        self.reg_map.append(
                {'off' : 0x6042,
                 'value' : 'default',
                 'name' : 'adc res',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['2k (800ns)', '4k (1.6us)', '4k (3.2us)',
                                     '8k (6.4us)', '8k (12.5us)', 'default'],
                 })
        # input range
        self.reg_map.append(
                {'off' : 0x6060,
                 'value' : 'default',
                 'name' : 'input range',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['4V', '10V', '8V', 'default'],
                 })
        # ecl gate1 osc
        self.reg_map.append(
                {'off' : 0x6064,
                 'value' : 'default',
                 'name' : 'ecl g1 osc',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['gate1 inp', 'osc inp', 'default'],
                 })
        # ecl fast clear and ts reset
        self.reg_map.append(
                {'off' : 0x6066,
                 'value' : 'default',
                 'name' : 'ecl fc res',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['fc inp', 'ts rst inp', 'default'],
                 })
        # nim gate1 osc
        self.reg_map.append(
                {'off' : 0x606a,
                 'value' : 'default',
                 'name' : 'nim g1 osc',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['gate1 inp', 'osc inp', 'default'],
                 })

        # nim fast clear and ts reset
        self.reg_map.append(
                {'off' : 0x606c,
                 'value' : 'default',
                 'name' : 'nim fc res',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['fc inp', 'ts rst inp', 'default'],
                 })
        # reset counters AB
        self.reg_map.append(
                {'off' : 0x6090,
                 'value' : 'default',
                 'name' : 'rst ctr',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # time stamp source
        self.reg_map.append(
                {'off' : 0x6096,
                 'value' : 'default',
                 'name' : 'ts src',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # time stamp div
        self.reg_map.append(
                {'off' : 0x6098,
                 'value' : 'default',
                 'name' : 'ts div',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # others has no corresponding widget in the settings area. We add them
        # in a loop:
#        for addr in range(0x6000, 0x60b0, 2):
#            if self._find_reg(addr)[0] >= 0:
#                continue
#            self.reg_map.append(
#                {'off' : addr,
#                 'value' : 'default',
#                 'name' : '0x%04x' % addr,
#                 'nbit' : 32,
#                 'has_set_wid' : False,
#                 'set_wid_type' : 'entry',
#                 })

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
