#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type


# this class is the advanced configuration for the logger

import tkinter as tk
from adv_conf import adv_conf 

class adv_conf_log(adv_conf):
    def __init__(self):
        self.var_lst = []

        # the trigger event ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'trig_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the scaler event ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'scal_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the message ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'msg_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The server address of the receiver thread
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ebd_server_addr',
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
        # The data saving path
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'save_path',
                 'str2int' : self._str2_big_int,
                 'int2str' : self._big_int2_str,
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The data saving buf len
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'save_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The contrler socket buffer size
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The contrler server address
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_svr_addr',
                 'str2int' : self._str2_big_int,
                 'int2str' : self._big_int2_str,
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The contrler time out of select syscall
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_t_us',
                 'wid_type' : 'entry',
                 'comment' : None
                 })


    def _str2_big_int(self, ch):
        try:
            tmp = ch.strip().lower()
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
