#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the v1190 class. This object manages the configuration data of a
# v1190A module
##########
# By B.Gao Sep. 2018

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class v1190(vme_mod):
    def __init__(self, name, mod = None):
        # we need to create the reg_map variable
        self.reg_map = []
        # interrupt level
        self.reg_map.append({
                 'off' : 0x100a,
                 'value' : 'default',
                 'name' : 'int lev',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['0','1','2','3','4','5','6','7','default']
                 })

        # interrupt vectro
        self.reg_map.append({
                 'off' : 0x100c,
                 'value' : 'default',
                 'name' : 'int vec',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # GEO
        self.reg_map.append({
                 'off' : 0x100e,
                 'value' : 'default',
                 'name' : 'geo',
                 'nbit' : 5,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # MCST base
        self.reg_map.append({
                 'off' : 0x1010,
                 'value' : 'default',
                 'name' : 'mcst base',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # MCST control
        self.reg_map.append({
                 'off' : 0x1012,
                 'value' : 'default',
                 'name' : 'mcst ctrl',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['disable', 'last', 'first', 'middle', 'default']
                 })

        # Almost full level:
        self.reg_map.append({
                 'off' : 0x1022,
                 'value' : 'default',
                 'name' : 'almost full',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # BLT event number:
        self.reg_map.append({
                 'off' : 0x1024,
                 'value' : 'default',
                 'name' : 'blt evt N',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # out prog control:
        self.reg_map.append({
                 'off' : 0x102c,
                 'value' : 'default',
                 'name' : 'out prog',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['data ready', 'full', 'almost full', 'error', 'default']
                 })

        # HPTDC settings (via the micro register), to make the management
        # easier, we also treat them as registers whoes address were explained
        # below.  because v1190 is different, its settings are not only about
        # registers, but also the micro registers. We organize the settings of
        # the micro registers also in the format of 'offset'-'value' pairs.
        # However, because there is not really offset for each setting, we
        # define them here artificially (arbitrarily). We only need to make the
        # definitions consistent.
        ##################################
        # offset    meaning                                     opcode
        # 0         acq mode                                    00xx/01xx
        #           (0->trigger matching, 1->continous)
        # 1         window width                                10xx
        # 2         window offset                               11xx
        # 3         extra search margin                         12xx        
        # 4         reject margin                               13xx
        # 5         subtraction of trigger time                 14xx/15xx
        #           (0->yes, 1->no)
        # 6         set detection                               22xx
        #           (0->pair, 1->trailing, 2->leanding,
        #            3->trail&lead)
        # 7         LSB of leading/trailing edge                24xx
        #           (800ps->0, 200ps->1, 100ps->2, 25ps->3)
        # 8         leading time and width res. when pair       25xx
        #           (100ps->0, 200ps->1, 400ps->2, 800ps->3, 
        #            1.6ns->4, 3.12ns->5, 6.25ns->6, 12.5ns->7)
        # 9         channel dead time between hits              28xx
        #           (5ns->0, 10ns->1, 30ns->2, 100ns->3)
        # 10        enable/disable tdc head and trailer         30xx/31xx
        #           (0->enable, 1->disable)
        # 11        max number of hits per event                33xx
        # 12        enable/disable TDC error mark               35xx
        #           (0->enable, 1->disable)
        # 13        enable/disable bypass TDC if error          37xx/38xx
        #           (0->enable, 1->disable)
        # 14        TDC internal error type                     39xx
        # 15        effective size of readout fifo              3Bxx
        #           (2wd->0, 4wd->1, 8wd->2, 16wd->3, 32wd->4, 
        #             64wd->5, 128wd->6, 256wd->7)
        # 16        channel enable mask (group 0)
        # 17        channel enable mask (group 1)
        # 18        channel enable mask (group 2)
        # 19        channel enable mask (group 3)
        # 20        global offset                               50xx
        # 100       channel adjust (channel 0)                  52nn
        # 101       channel adjust (channel 1)                  52nn
        # ...       ............                                ...
        # 227       channel adjust (channel 127)                52nn
        # Add other if necessary...

        # acq mode (trigger matching or continous)
        self.reg_map.append({
                 'off' : 0,
                 'value' : 'default',
                 'name' : 'acq mode',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['trig', 'cont', 'default']
                 })
        
        # window width:
        self.reg_map.append({
                 'off' : 1,
                 'value' : 'default',
                 'name' : 'window width',
                 'nbit' : 12,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # window offset:
        self.reg_map.append({
                 'off' : 2,
                 'value' : 'default',
                 'name' : 'window offset',
                 'nbit' : 12,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # extra search margin
        self.reg_map.append({
                 'off' : 3,
                 'value' : 'default',
                 'name' : 'ex search',
                 'nbit' : 12,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # reject margin
        self.reg_map.append({
                 'off' : 4,
                 'value' : 'default',
                 'name' : 'reject',
                 'nbit' : 12,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # subtraction trigger time
        self.reg_map.append({
                 'off' : 5,
                 'value' : 'default',
                 'name' : 'sub trig T',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['yes', 'no', 'default']
                 })
        # edge detection 
        # (pair->0, trailing->1, leading->2, trailing and leading->3)
        self.reg_map.append({
                 'off' : 6,
                 'value' : 'default',
                 'name' : 'edge detect',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['pair', 'trailing', 'leading', 
                     'trai&lead', 'default']
                 })
        # LSB of tr/lead edge
        # (800ps->0, 200ps->1, 100ps->2, 25ps->3)
        self.reg_map.append({
                 'off' : 7,
                 'value' : 'default',
                 'name' : 'LSB',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['800ps', '200ps', '100ps', '25ps', 
                     'default']
                 })
        # leading time and width resolution when pair:
        # 100ps->0, 200ps->1, 400ps->2, 800ps->3, 1.6ns->4, 3.12ns->5,
        # 6.25ns->6, 12.5ns->7
        self.reg_map.append({
                 'off' : 8,
                 'value' : 'default',
                 'name' : 'pair res.',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['100ps', '200ps', '400ps', '800ps',
                     '1.6ns', '3.12ns', '6.25ns', '12.5ns', 'default']
                 })
        # channel dead time between hits:
        # 5ns->0, 10ns->1, 30ns->2, 100ns->3
        self.reg_map.append({
                 'off' : 9,
                 'value' : 'default',
                 'name' : 'dead t',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['5ns', '10ns', '30ns', '100ns', 'default'] 
                 })
        # TDC header and trailer:
        self.reg_map.append({
                 'off' : 10,
                 'value' : 'default',
                 'name' : 'tdc h&tr',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['enable', 'disable', 'default']
                 })
        # max hit per event
        self.reg_map.append({
                 'off' : 11,
                 'value' : 'default',
                 'name' : 'max hit',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # tdc error mark
        self.reg_map.append({
                 'off' : 12,
                 'value' : 'default',
                 'name' : 'tdc err mark',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['enable', 'disable', 'default']
                 })
        # by tdc if error
        self.reg_map.append({
                 'off' : 13,
                 'value' : 'default',
                 'name' : 'tdc bypass',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['enable', 'disable', 'default']
                 })
        # effective size of fifo
        # 2wd->0, 4wd->1, 8wd->2, 16wd->3, 32wd->4, 64wd->5, 128wd->6, 256wd->7
        self.reg_map.append({
                 'off' : 15,
                 'value' : 'default',
                 'name' : 'fifo size',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['2wd', '4wd', '8wd', '16wd', '32wd',
                     '64wd', '128wd', '256wd', 'default']
                 })
        # channel enable pattern
        for i in range(4):
            self.reg_map.append({
                     'off' : 16+i,
                     'value' : 'default',
                     'name' : 'enable msk%d' % i,
                     'nbit' : 32,
                     'has_set_wid' : True,
                     'set_wid_type' : 'entry',
                     })
        # set global offset
        self.reg_map.append({
                 'off' : 20,
                 'value' : 'default',
                 'name' : 'global off',
                 'nbit' : 17,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # channel offset
        self.ch_off_w = []
        for i in range(128):
            self.reg_map.append({
                     'off' : 100+i,
                     'value' : 'default',
                     'name' : 'ch%03d off' % i,
                     'nbit' : 8,
                     'has_set_wid' : True,
                     'set_wid_type' : 'entry',
                     })
        
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
                 'off' : 500,
                 'value' : 'default',
                 'name' : 'ctrl bit set',
                 'nbit' : 13,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        self.reg_map.append({
                 'off' : 501,
                 'value' : 'default',
                 'name' : 'ctrl bit clr',
                 'nbit' : 13,
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
#v1190 = v1190('v1190')
#tk.Button(win, text='button', command=v1190.show_win).pack()
#win.mainloop()
