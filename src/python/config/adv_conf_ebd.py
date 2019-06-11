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
        # The server address of the receiver thread
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'fe_server_addr',
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




########## TEST ##########
#win = tk.Tk()
#adv_conf = adv_conf_fe()
#tk.Button(win, text='button', command=adv_conf.show_win).pack()
#win.mainloop()
