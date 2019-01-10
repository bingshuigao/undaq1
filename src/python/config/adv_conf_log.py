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




########## TEST ##########
#win = tk.Tk()
#adv_conf = adv_conf_fe()
#tk.Button(win, text='button', command=adv_conf.show_win).pack()
#win.mainloop()
