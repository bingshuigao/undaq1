#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the mtdc32 class. This object manages the configuration data of a
# mtdc32 module
###########3
# By B.Gao Sep. 2023

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class mtdc32(vme_mod):
    def __init__(self, name, mod=None):
        # we need to create the reg_map variable
        self.reg_map = []
        # module id
        self.reg_map.append(
                {'off' : 0x6004,
                 'value' : 0,
                 'name' : 'module id',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # max transfer data
        self.reg_map.append(
                {'off' : 0x601a,
                 'value' : 200,
                 'name' : 'max trans data',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
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
                 'value' : 3,
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
                 'value' : 0,
                 'name' : 'marking type',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['evt cnt', 'ts', 'unknown', 'ext ts', 
                                     'default'],
                 })
        # tdc resolution
        self.reg_map.append(
                {'off' : 0x6042,
                 'value' : 2,
                 'name' : 'tdc res',
                 'nbit' : 5,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['0->N/A',
                                     '1->N/A',
                                     '2->3.9  ps',
                                     '3->7.8  ps',
                                     '4->15.6 ps',
                                     '5->31.3 ps',
                                     '6->62.5 ps',
                                     '7->125  ps',
                                     '8->250  ps',
                                     '9->500  ps',
                                     'default'],
                 })
        # win start 0
        self.reg_map.append(
                {'off' : 0x6050,
                 'value' : 16384,
                 'name' : 'win start 0',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # win start 1
        self.reg_map.append(
                {'off' : 0x6052,
                 'value' : 16384,
                 'name' : 'win start 1',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # win windth 0
        self.reg_map.append(
                {'off' : 0x6054,
                 'value' : 32,
                 'name' : 'win width 0',
                 'nbit' : 14,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # win windth 1
        self.reg_map.append(
                {'off' : 0x6056,
                 'value' : 32,
                 'name' : 'win width 1',
                 'nbit' : 14,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # bank0 trig src
        self.reg_map.append(
                {'off' : 0x6058,
                 'value' : 'default',
                 'name' : 'bk0 trig src',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # bank1 trig src
        self.reg_map.append(
                {'off' : 0x605a,
                 'value' : 'default',
                 'name' : 'bk1 trig src',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # if first hit only
        self.reg_map.append(
                {'off' : 0x605c,
                 'value' : 'default',
                 'name' : 'first hit',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # negtive edge
        self.reg_map.append(
                {'off' : 0x6060,
                 'value' : 'default',
                 'name' : 'neg edge',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # ecl term 
        self.reg_map.append(
                {'off' : 0x6062,
                 'value' : 'default',
                 'name' : 'ECL term',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # ecl gate1 osc
        self.reg_map.append(
                {'off' : 0x6064,
                 'value' : 'default',
                 'name' : 'ecl trig1 osc',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['trig1 inp', 'osc inp', 'default'],
                 })
        # trigger select
        self.reg_map.append(
                {'off' : 0x6068,
                 'value' : 'default',
                 'name' : 'trig sel',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['NIM inp', 'ECL inp', 'default'],
                 })
        # nim gate1 osc
        self.reg_map.append(
                {'off' : 0x606a,
                 'value' : 1,
                 'name' : 'nim trig1 osc',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['trig1 inp', 'osc inp', 'default'],
                 })

        # pulser status
        self.reg_map.append(
                {'off' : 0x6070,
                 'value' : 'default',
                 'name' : 'pulse stat',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['off', 'on', 'default'],
                 })
        # pulser pattern
        self.reg_map.append(
                {'off' : 0x6072,
                 'value' : 'default',
                 'name' : 'pulse pat',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # bank0 input threshold
        self.reg_map.append(
                {'off' : 0x6078,
                 'value' : 'default',
                 'name' : 'bk0 inp thr',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # bank1 input threshold
        self.reg_map.append(
                {'off' : 0x607a,
                 'value' : 'default',
                 'name' : 'bk1 inp thr',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })


        # reset counters AB
        self.reg_map.append(
                {'off' : 0x6090,
                 'value' : 12,
                 'name' : 'rst ctr',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # time stamp source
        self.reg_map.append(
                {'off' : 0x6096,
                 'value' : 3,
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

        # bank0 multiplicity upper limit
        self.reg_map.append(
                {'off' : 0x60b0,
                 'value' : 'default',
                 'name' : 'bk0 mult high',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # bank0 multiplicity lower limit
        self.reg_map.append(
                {'off' : 0x60b2,
                 'value' : 'default',
                 'name' : 'bk0 mult low',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # bank1 multiplicity upper limit
        self.reg_map.append(
                {'off' : 0x60b4,
                 'value' : 'default',
                 'name' : 'bk1 mult high',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # bank1 multiplicity lower limit
        self.reg_map.append(
                {'off' : 0x60b6,
                 'value' : 'default',
                 'name' : 'bk1 mult low',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        self.geo_addr = 0x6004
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
