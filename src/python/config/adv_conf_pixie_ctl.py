#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type


# this class is the advanced configuration for the pixie16 contoller modules

import tkinter as tk
from adv_conf import adv_conf 

class adv_conf_pixie_ctl(adv_conf):
    def __init__(self):
        self.var_lst = []

        # if it is the trigger module
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'pixie16_ctl1',
                 'wid_type' : 'comb',
                 'wid_values' : ['No', 'Yes', 'default'],
                 'comment' : None
                 })
        # clock frequency (in Hz)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'pixie16_ctl2',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # clock offset (in clock ticks)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'pixie16_ctl3',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The dsp parameter file
        self.var_lst.append({
                 'value' : 0x617461642f2e2e,
                 'name' : 'dsp set file',
                 'str2int' : self._str2_big_int,
                 'int2str' : self._big_int2_str,
                 'wid_type' : 'entry',
                 'comment' : None
                 })


    def _str2_big_int(self, ch):
        try:
            tmp = ch.strip()
            if tmp == 'default':
                return 0, 'DEF'
            big_int = 0 
            for (i,x) in enumerate(tmp):
                big_int += ord(x) << (i*8)                                                                                                                   
        except:
            return 0, 'ERR'
        return big_int, 'OK'
    
    def _big_int2_str(self, big_int):
        hex_str = '%x' % big_int
        str_out = ''
        for i in range(len(hex_str)//2):
            sub_str = '0x' + hex_str[i*2:i*2+2]
            str_out = '%c' % int(sub_str, 16) + str_out
        return str_out


########## TEST ##########
#win = tk.Tk()
#adv_conf = adv_conf_fe()
#tk.Button(win, text='button', command=adv_conf.show_win).pack()
#win.mainloop()
