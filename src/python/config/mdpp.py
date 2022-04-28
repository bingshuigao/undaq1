#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the mdpp class. This object manages the configuration data of a
# mdpp module
###########3
# By B.Gao Apr. 2022

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class mdpp(vme_mod):
    def __init__(self, name, mod=None):
        # we need to create the reg_map variable
        self.reg_map = []
        # firmware
        self.reg_map.append(
                {'off' : 0x1,
                 'value' : 1,
                 'name' : 'soft mod',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['RCP', 'SCP'],
                 })
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
                 'value' : 5,
                 'name' : 'adc res',
                 'nbit' : 3,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['24 ps', '49 ps', '98 ps',
                                     '195 ps', '391 ps', '781 ps', 'default'],
                 })
        # output format 
        self.reg_map.append(
                {'off' : 0x6044,
                 'value' : 0,
                 'name' : 'out fmt',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['std', 'amp', 'time', 'default'],
                 })
        # window start
        self.reg_map.append(
                {'off' : 0x6050,
                 'value' : 16384,
                 'name' : 'win start',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # window width
        self.reg_map.append(
                {'off' : 0x6054,
                 'value' : 32,
                 'name' : 'win width',
                 'nbit' : 14,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # trigger source
        self.reg_map.append(
                {'off' : 0x6058,
                 'value' : 0x100,
                 'name' : 'trig src',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # trigger source
        self.reg_map.append(
                {'off' : 0x605a,
                 'value' : 0,
                 'name' : 'trig src2',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # first hit
        self.reg_map.append(
                {'off' : 0x605c,
                 'value' : 1,
                 'name' : '1st hit',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # trigger output
        self.reg_map.append(
                {'off' : 0x605e,
                 'value' : 0x100,
                 'name' : 'trig out',
                 'nbit' : 10,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # ECL3 
        self.reg_map.append(
                {'off' : 0x6060,
                 'value' : 0,
                 'name' : 'ECL3',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # ECL2 
        self.reg_map.append(
                {'off' : 0x6062,
                 'value' : 0,
                 'name' : 'ECL2',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # ECL1 
        self.reg_map.append(
                {'off' : 0x6064,
                 'value' : 0,
                 'name' : 'ECL1',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # ECL0 
        self.reg_map.append(
                {'off' : 0x6066,
                 'value' : 0,
                 'name' : 'ECL0',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 'get_set_wid_val' : self._get_val_hex,
                 })
        # NIM4
        self.reg_map.append(
                {'off' : 0x6068,
                 'value' : 1,
                 'name' : 'NIM4',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # NIM3
        self.reg_map.append(
                {'off' : 0x606a,
                 'value' : 1,
                 'name' : 'NIM3',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # NIM2
        self.reg_map.append(
                {'off' : 0x606c,
                 'value' : 1,
                 'name' : 'NIM2',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # NIM0
        self.reg_map.append(
                {'off' : 0x6074,
                 'value' : 1,
                 'name' : 'NIM0',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pulser status
        self.reg_map.append(
                {'off' : 0x6070,
                 'value' : 0,
                 'name' : 'pul stat',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pulser amplitude
        self.reg_map.append(
                {'off' : 0x6072,
                 'value' : 400,
                 'name' : 'pul amp',
                 'nbit' : 12,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # monitor on
        self.reg_map.append(
                {'off' : 0x607a,
                 'value' : 0,
                 'name' : 'monitor',
                 'nbit' : 1,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # monitor channel
        self.reg_map.append(
                {'off' : 0x607c,
                 'value' : 0,
                 'name' : 'mon ch',
                 'nbit' : 4,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # monitor wave
        self.reg_map.append(
                {'off' : 0x607e,
                 'value' : 0,
                 'name' : 'mon wav',
                 'nbit' : 2,
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
        # multiplicity filter
        self.reg_map.append(
                {'off' : 0x60b0,
                 'value' : 255,
                 'name' : 'mult hi',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # multiplicity filter
        self.reg_map.append(
                {'off' : 0x60b2,
                 'value' : 0,
                 'name' : 'mult lo',
                 'nbit' : 8,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # TF int diff
        self.reg_map.append(
                {'off' : 0x8000,
                 'value' : 20,
                 'name' : 'TF g0',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # TF int diff
        self.reg_map.append(
                {'off' : 0x8001,
                 'value' : 20,
                 'name' : 'TF g1',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # TF int diff
        self.reg_map.append(
                {'off' : 0x8002,
                 'value' : 20,
                 'name' : 'TF g2',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # TF int diff
        self.reg_map.append(
                {'off' : 0x8003,
                 'value' : 20,
                 'name' : 'TF g3',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # TF int diff
        self.reg_map.append(
                {'off' : 0x8004,
                 'value' : 20,
                 'name' : 'TF g4',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # TF int diff
        self.reg_map.append(
                {'off' : 0x8005,
                 'value' : 20,
                 'name' : 'TF g5',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # TF int diff
        self.reg_map.append(
                {'off' : 0x8006,
                 'value' : 20,
                 'name' : 'TF g6',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # TF int diff
        self.reg_map.append(
                {'off' : 0x8007,
                 'value' : 20,
                 'name' : 'TF g7',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # gain
        self.reg_map.append(
                {'off' : 0x8010,
                 'value' : 2000,
                 'name' : 'Gain g0',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # gain
        self.reg_map.append(
                {'off' : 0x8011,
                 'value' : 2000,
                 'name' : 'Gain g1',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # gain
        self.reg_map.append(
                {'off' : 0x8012,
                 'value' : 2000,
                 'name' : 'Gain g2',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # gain
        self.reg_map.append(
                {'off' : 0x8013,
                 'value' : 2000,
                 'name' : 'Gain g3',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # gain
        self.reg_map.append(
                {'off' : 0x8014,
                 'value' : 2000,
                 'name' : 'Gain g4',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # gain
        self.reg_map.append(
                {'off' : 0x8015,
                 'value' : 2000,
                 'name' : 'Gain g5',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # gain
        self.reg_map.append(
                {'off' : 0x8016,
                 'value' : 2000,
                 'name' : 'Gain g6',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # gain
        self.reg_map.append(
                {'off' : 0x8017,
                 'value' : 2000,
                 'name' : 'Gain g7',
                 'nbit' : 15,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # shaping time
        self.reg_map.append(
                {'off' : 0x8020,
                 'value' : 160,
                 'name' : 'shape g0',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # shaping time
        self.reg_map.append(
                {'off' : 0x8021,
                 'value' : 160,
                 'name' : 'shape g1',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # shaping time
        self.reg_map.append(
                {'off' : 0x8022,
                 'value' : 160,
                 'name' : 'shape g2',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # shaping time
        self.reg_map.append(
                {'off' : 0x8023,
                 'value' : 160,
                 'name' : 'shape g3',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # shaping time
        self.reg_map.append(
                {'off' : 0x8024,
                 'value' : 160,
                 'name' : 'shape g4',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # shaping time
        self.reg_map.append(
                {'off' : 0x8025,
                 'value' : 160,
                 'name' : 'shape g5',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # shaping time
        self.reg_map.append(
                {'off' : 0x8026,
                 'value' : 160,
                 'name' : 'shape g6',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # shaping time
        self.reg_map.append(
                {'off' : 0x8027,
                 'value' : 160,
                 'name' : 'shape g7',
                 'nbit' : 11,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # reset time
        self.reg_map.append(
                {'off' : 0x8030,
                 'value' : 1000,
                 'name' : 'rst t g0',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # reset time
        self.reg_map.append(
                {'off' : 0x8031,
                 'value' : 1000,
                 'name' : 'rst t g1',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # reset time
        self.reg_map.append(
                {'off' : 0x8032,
                 'value' : 1000,
                 'name' : 'rst t g2',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # reset time
        self.reg_map.append(
                {'off' : 0x8033,
                 'value' : 1000,
                 'name' : 'rst t g3',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # reset time
        self.reg_map.append(
                {'off' : 0x8034,
                 'value' : 1000,
                 'name' : 'rst t g4',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # reset time
        self.reg_map.append(
                {'off' : 0x8035,
                 'value' : 1000,
                 'name' : 'rst t g5',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # reset time
        self.reg_map.append(
                {'off' : 0x8036,
                 'value' : 1000,
                 'name' : 'rst t g6',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # reset time
        self.reg_map.append(
                {'off' : 0x8037,
                 'value' : 1000,
                 'name' : 'rst t g7',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # rise time
        self.reg_map.append(
                {'off' : 0x8040,
                 'value' : 80,
                 'name' : 'rise t g0',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # rise time
        self.reg_map.append(
                {'off' : 0x8041,
                 'value' : 80,
                 'name' : 'rise t g1',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # rise time
        self.reg_map.append(
                {'off' : 0x8042,
                 'value' : 80,
                 'name' : 'rise t g2',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # rise time
        self.reg_map.append(
                {'off' : 0x8043,
                 'value' : 80,
                 'name' : 'rise t g3',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # rise time
        self.reg_map.append(
                {'off' : 0x8044,
                 'value' : 80,
                 'name' : 'rise t g4',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # rise time
        self.reg_map.append(
                {'off' : 0x8045,
                 'value' : 80,
                 'name' : 'rise t g5',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # rise time
        self.reg_map.append(
                {'off' : 0x8046,
                 'value' : 80,
                 'name' : 'rise t g6',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # rise time
        self.reg_map.append(
                {'off' : 0x8047,
                 'value' : 80,
                 'name' : 'rise t g7',
                 'nbit' : 7,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8050,
                 'value' : 255,
                 'name' : 'thre ch0',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8051,
                 'value' : 255,
                 'name' : 'thre ch1',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8052,
                 'value' : 255,
                 'name' : 'thre ch2',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8053,
                 'value' : 255,
                 'name' : 'thre ch3',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8054,
                 'value' : 255,
                 'name' : 'thre ch4',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8055,
                 'value' : 255,
                 'name' : 'thre ch5',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8056,
                 'value' : 255,
                 'name' : 'thre ch6',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8057,
                 'value' : 255,
                 'name' : 'thre ch7',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8058,
                 'value' : 255,
                 'name' : 'thre ch8',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x8059,
                 'value' : 255,
                 'name' : 'thre ch9',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x805a,
                 'value' : 255,
                 'name' : 'thre ch10',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x805b,
                 'value' : 255,
                 'name' : 'thre ch11',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x805c,
                 'value' : 255,
                 'name' : 'thre ch12',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x805d,
                 'value' : 255,
                 'name' : 'thre ch13',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x805e,
                 'value' : 255,
                 'name' : 'thre ch14',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # threshold
        self.reg_map.append(
                {'off' : 0x805f,
                 'value' : 255,
                 'name' : 'thre ch15',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8060,
                 'value' : 65535,
                 'name' : 'pz ch0',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8061,
                 'value' : 65535,
                 'name' : 'pz ch1',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8062,
                 'value' : 65535,
                 'name' : 'pz ch2',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8063,
                 'value' : 65535,
                 'name' : 'pz ch3',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8064,
                 'value' : 65535,
                 'name' : 'pz ch4',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8065,
                 'value' : 65535,
                 'name' : 'pz ch5',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8066,
                 'value' : 65535,
                 'name' : 'pz ch6',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8067,
                 'value' : 65535,
                 'name' : 'pz ch7',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8068,
                 'value' : 65535,
                 'name' : 'pz ch8',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x8069,
                 'value' : 65535,
                 'name' : 'pz ch9',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x806a,
                 'value' : 65535,
                 'name' : 'pz ch10',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x806b,
                 'value' : 65535,
                 'name' : 'pz ch11',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x806c,
                 'value' : 65535,
                 'name' : 'pz ch12',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x806d,
                 'value' : 65535,
                 'name' : 'pz ch13',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x806e,
                 'value' : 65535,
                 'name' : 'pz ch14',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # pole zero
        self.reg_map.append(
                {'off' : 0x806f,
                 'value' : 65535,
                 'name' : 'pz ch15',
                 'nbit' : 16,
                 'has_set_wid' : True,
                 'set_wid_type' : 'entry',
                 })
        # base line restore 
        self.reg_map.append(
                {'off' : 0x8070,
                 'value' : 2,
                 'name' : 'BLR g0',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['off', 'strict', 'soft'],
                 })
        # base line restore 
        self.reg_map.append(
                {'off' : 0x8071,
                 'value' : 2,
                 'name' : 'BLR g1',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['off', 'strict', 'soft'],
                 })
        # base line restore 
        self.reg_map.append(
                {'off' : 0x8072,
                 'value' : 2,
                 'name' : 'BLR g2',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['off', 'strict', 'soft'],
                 })
        # base line restore 
        self.reg_map.append(
                {'off' : 0x8073,
                 'value' : 2,
                 'name' : 'BLR g3',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['off', 'strict', 'soft'],
                 })
        # base line restore 
        self.reg_map.append(
                {'off' : 0x8074,
                 'value' : 2,
                 'name' : 'BLR g4',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['off', 'strict', 'soft'],
                 })
        # base line restore 
        self.reg_map.append(
                {'off' : 0x8075,
                 'value' : 2,
                 'name' : 'BLR g5',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['off', 'strict', 'soft'],
                 })
        # base line restore 
        self.reg_map.append(
                {'off' : 0x8076,
                 'value' : 2,
                 'name' : 'BLR g6',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['off', 'strict', 'soft'],
                 })
        # base line restore 
        self.reg_map.append(
                {'off' : 0x8077,
                 'value' : 2,
                 'name' : 'BLR g7',
                 'nbit' : 2,
                 'has_set_wid' : True,
                 'set_wid_type' : 'comb',
                 'set_wid_values' : ['off', 'strict', 'soft'],
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
#adc32 = mdpp('adc32')
#tk.Button(win, text='button', command=adc32.show_win).pack()
#win.mainloop()
