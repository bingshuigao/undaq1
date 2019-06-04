#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the v1740 class. This object manages the configuration data of a
# v1740 module
###########3
# By B.Gao Jun. 2019

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class v1740(vme_mod):
    def __init__(self, name1, mod=None):
        # we need to create the reg_map variable
        self.reg_map = []
        # group n trigger threashold
        for i in range(8):
            off = 0x1080 + 0x100*i
            name = 'trig_th_G%d' % i
            self.reg_map.append(
                {'off' : off,
                 'value' : 'default',
                 'name' : name,
                 'nbit' : 12,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # group n DC offset
        for i in range(8):
            off = 0x1098 + 0x100*i
            name = 'DC_off_G%d' % i
            self.reg_map.append(
                {'off' : off,
                 'value' : 'default',
                 'name' : name,
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # group n channel enable mask
        for i in range(8):
            off = 0x10a8 + 0x100*i
            name = 'enable_msk_G%d' % i
            self.reg_map.append(
                {'off' : off,
                 'value' : 'default',
                 'name' : name,
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # group n low channels DC offset individual correction
        for i in range(8):
            off = 0x10c0 + 0x100*i
            name = 'low_DC_G%d' % i
            self.reg_map.append(
                {'off' : off,
                 'value' : 'default',
                 'name' : name,
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # group n high channels DC offset individual correction
        for i in range(8):
            off = 0x10c4 + 0x100*i
            name = 'hi_DC_G%d' % i
            self.reg_map.append(
                {'off' : off,
                 'value' : 'default',
                 'name' : name,
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # board config 
        self.reg_map.append(
                {'off' : 0x8000,
                 'value' : 'default',
                 'name' : 'bd conf',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # buffer organization 
        self.reg_map.append(
                {'off' : 0x800c,
                 'value' : 'default',
                 'name' : 'Nr. buf',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb', 
                 'set_wid_values' : ['1','2','4','8','16','32','64','128','256','512','1024','default'],
                 })
        # custom size
        self.reg_map.append(
                {'off' : 0x8020,
                 'value' : 'default',
                 'name' : 'cstm sz',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # acqusation control
        self.reg_map.append(
                {'off' : 0x8100,
                 'value' : 'default',
                 'name' : 'acq ctl',
                 'nbit' : 13,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # global trigger mask
        self.reg_map.append(
                {'off' : 0x810c,
                 'value' : 'default',
                 'name' : 'trig msk',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # trigger out enable mask
        self.reg_map.append(
                {'off' : 0x8110,
                 'value' : 'default',
                 'name' : 'GPO en msk',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # post trigger
        self.reg_map.append(
                {'off' : 0x8114,
                 'value' : 'default',
                 'name' : 'post trig',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # front panel io control
        self.reg_map.append(
                {'off' : 0x811c,
                 'value' : 'default',
                 'name' : 'IO ctl',
                 'nbit' : 23,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # group enable mask
        self.reg_map.append(
                {'off' : 0x8120,
                 'value' : 'default',
                 'name' : 'grp en msk',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # voltage level mode config
        self.reg_map.append(
                {'off' : 0x8138,
                 'value' : 'default',
                 'name' : 'VL mod',
                 'nbit' : 12,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # analog monitor mode
        self.reg_map.append(
                {'off' : 0x8144,
                 'value' : 'default',
                 'name' : 'ana mon mod',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb', 
                 'set_wid_values' : ['trig maj','test','resv','buf occu','vol lev','default'],
                 })
        # buffer almost full level
        self.reg_map.append(
                {'off' : 0x816c,
                 'value' : 'default',
                 'name' : 'almst ful',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # run/start/stop delay
        self.reg_map.append(
                {'off' : 0x8170,
                 'value' : 'default',
                 'name' : 'r/s/s delay',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # front panel i/o new features
        self.reg_map.append(
                {'off' : 0x81a0,
                 'value' : 'default',
                 'name' : 'i/o new fea',
                 'nbit' : 17,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # buffer occupancy gain
        self.reg_map.append(
                {'off' : 0x81b4,
                 'value' : 'default',
                 'name' : 'buf occ gain',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # extended veto delay
        self.reg_map.append(
                {'off' : 0x81c4,
                 'value' : 'default',
                 'name' : 'ext veto dly',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # readout control
        self.reg_map.append(
                {'off' : 0xef00,
                 'value' : 'default',
                 'name' : 'rd ctl',
                 'nbit' : 9,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # board id
        self.reg_map.append(
                {'off' : 0xef08,
                 'value' : 'default',
                 'name' : 'bd id',
                 'nbit' : 5,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # BLT base
        self.reg_map.append(
                {'off' : 0xef0c,
                 'value' : 'default',
                 'name' : 'blt ctl',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # interrupt id
        self.reg_map.append(
                {'off' : 0xef14,
                 'value' : 'default',
                 'name' : 'int vec',
                 'nbit' : 32,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # interrupt threshold
        self.reg_map.append(
                {'off' : 0xef18,
                 'value' : 'default',
                 'name' : 'int thr',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })
        # max event number in a blt
        self.reg_map.append(
                {'off' : 0xef1c,
                 'value' : 'default',
                 'name' : 'max blt N',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry', 
                 })

        # This method must be called at the **end**.
        self._base_init(name1, mod)
    
    # get the value of the register in hexdecimal format
    def _get_val_hex(self, reg):
        val = reg['value']
        if val == 'default':
            return 'default'
        return '0x%08x' % val

# test ##########
#win = tk.Tk()
#adc32 = madc32('adc32')
#tk.Button(win, text='button', command=adc32.show_win).pack()
#win.mainloop()
