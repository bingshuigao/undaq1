#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the v2718. This object manages the configuration data of a v2718
# module
###########3
# By B.Gao Sep. 2018

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class v2718(vme_mod):
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
                 'off' : 500,
                 'value' : 'default',
                 'name' : 'ctrl bit set',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        self.reg_map.append({
                 'off' : 501,
                 'value' : 'default',
                 'name' : 'ctrl bit clr',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # interrupt mask
        self.reg_map.append({
                 'off' : 0x06,
                 'value' : 'default',
                 'name' : 'irq mask',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })

        # output set/clr
        self.reg_map.append({
                 'off' : 0x0a,
                 'value' : 'default',
                 'name' : 'output set',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        self.reg_map.append({
                 'off' : 0x10,
                 'value' : 'default',
                 'name' : 'output clr',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # input multiplexer set/clr
        self.reg_map.append({
                 'off' : 0x0b,
                 'value' : 'default',
                 'name' : 'input mulp set',
                 'nbit' : 12,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        self.reg_map.append({
                 'off' : 0x11,
                 'value' : 'default',
                 'name' : 'input mulp clr',
                 'nbit' : 12,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # output multiplexer set/clr
        self.reg_map.append({
                 'off' : 0x0c,
                 'value' : 0xf,
                 'name' : 'output mulp set',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        self.reg_map.append({
                 'off' : 0x12,
                 'value' : 'default',
                 'name' : 'output mulp clr',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # LED polarity set/clr
        self.reg_map.append({
                 'off' : 0x0d,
                 'value' : 'default',
                 'name' : 'led pol set',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        self.reg_map.append({
                 'off' : 0x13,
                 'value' : 'default',
                 'name' : 'led pol clr',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # pulser A 0 register
        self.reg_map.append({
                 'off' : 0x16,
                 'value' : 'default',
                 'name' : 'pulser a0',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # pulser A 1 register
        self.reg_map.append({
                 'off' : 0x17,
                 'value' : 'default',
                 'name' : 'pulser a1',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # pulser B 0 register
        self.reg_map.append({
                 'off' : 0x19,
                 'value' : 'default',
                 'name' : 'pulser b0',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # pulser B 1 register
        self.reg_map.append({
                 'off' : 0x1a,
                 'value' : 'default',
                 'name' : 'pulser b1',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # scaler 0 register
        self.reg_map.append({
                 'off' : 0x1c,
                 'value' : 'default',
                 'name' : 'scaler 0',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # location monitor address low/high
        self.reg_map.append({
                 'off' : 0x28,
                 'value' : 'default',
                 'name' : 'LMON_AD_Lo',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        self.reg_map.append({
                 'off' : 0x29,
                 'value' : 'default',
                 'name' : 'LMON_AD_Hi',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })

        # location monitor control
        self.reg_map.append({
                 'off' : 0x2c,
                 'value' : 'default',
                 'name' : 'LMON_CTRL',
                 'nbit' : 9,
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
#v1190 = v2718('v2718')
#tk.Button(win, text='button', command=v1190.show_win).pack()
#win.mainloop()
