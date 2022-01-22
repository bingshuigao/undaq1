#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type


# this class is the advanced configuration for the event builder

import tkinter as tk
from adv_conf import adv_conf 

class adv_conf_ebd(adv_conf):
    def __init__(self):
        self.var_lst = []

        # the data ring buffer size for each module (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ring_bufs_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the message ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ring_buf2_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the built event ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ring_buf3_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the built event ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ring_buf5_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the raw data received from frontend ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ring_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the listening port of the event builder data sender 
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'sender_port',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The interval which the event builder data sender spend on checking
        # the ring buffer.
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'sender_itv',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The event builder socket buffer size (for the sender, in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'sender_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The server address of the frontend sender. Multiple addresses can be
        # specified here, separated by commas. Each address corresponds to a
        # frontend.
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'fe_server_addr',
                 'str2int' : self._str2_big_int,
                 'int2str' : self._big_int2_str,
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The timeout (us) of the receiver thread to check the socket 
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'recv_t_us',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The clock frequency 
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'sort_freq',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The glom
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'merge_glom',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The merged event buffer size
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'merge_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The control thread buffer size
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The control GUI server address
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_svr_addr',
                 'str2int' : self._str2_big_int,
                 'int2str' : self._big_int2_str,
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The control GUI server address
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_t_us',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The max length (32-byte words) of a single event
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'max_evt_len',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The event builder type (ts/evt cnt)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'merge_type',
                 'wid_type' : 'comb',
                 'wid_values' : ['default', 'ts', 'evt cnt'],
                 'comment' : None
                 })
        # The event builder type (ts/evt cnt)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'pixie_clk',
                 'wid_type' : 'comb',
                 'wid_values' : ['default', 'INT', 'EXT'],
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
