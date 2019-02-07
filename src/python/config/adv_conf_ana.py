#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type


# this class is the advanced configuration for the analyzer

import tkinter as tk
from adv_conf import adv_conf 

class adv_conf_ana(adv_conf):
    def __init__(self):
        self.var_lst = []

        # the event ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'trig_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'msg_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'scal_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the server address of the receiver
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ebd_server_addr',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the timeout of the select syscall
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'recv_t_us',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the max event length
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'main_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the roody server port
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'roody_svr_port',
                 'wid_type' : 'entry',
                 'comment' : None
                 })




########## TEST ##########
#win = tk.Tk()
#adv_conf = adv_conf_fe()
#tk.Button(win, text='button', command=adv_conf.show_win).pack()
#win.mainloop()
